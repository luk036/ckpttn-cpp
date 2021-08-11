#include <ckpttn/netlist.hpp>
#include <climits>
#include <fstream>
#include <iostream>
// #include <py2cpp/py2cpp.hpp>
#include <string_view>
#include <utility>  // for std::pair
#include <vector>

// using graph_t =
//     boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
// using node_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
// using edge_t = typename boost::graph_traits<graph_t>::edge_iterator;

using std::ifstream;
using std::ofstream;

// Read the IBM .netD/.net format. Precondition: Netlist is empty.
void writeJSON(std::string_view jsonFileName, const SimpleNetlist& H) {
    auto json = ofstream{jsonFileName.data()};
    if (json.fail()) {
        std::cerr << "Error: Can't open file " << jsonFileName << ".\n";
        exit(1);
    }
    json << R"({
 "directed": false,
 "multigraph": false,
 "graph": {
)";

    json << R"( "num_modules": )" << H.number_of_modules() << ",\n";
    json << R"( "num_nets": )" << H.number_of_nets() << ",\n";
    json << R"( "num_pads": )" << H.num_pads << "\n";
    json << " },\n";

    json << R"( "nodes": [)"
         << "\n";
    for (const auto& node : H.G) {
        json << "  { \"id\": " << node << " },\n";
    }
    json << " ],\n";

    json << R"( "links": [)"
         << "\n";
    for (const auto& v : H) {
        for (const auto& net : H.G[v]) {
            json << "  {\n";
            json << "   \"source\": " << v << ",\n";
            json << "   \"target\": " << net << "\n";
            json << "  },\n";
        }
    }
    json << " ]\n";

    json << "}\n";
}

// Read the IBM .netD/.net format. Precondition: Netlist is empty.
auto readNetD(std::string_view netDFileName) -> SimpleNetlist {
    auto netD = ifstream{netDFileName.data()};
    if (netD.fail()) {
        std::cerr << "Error: Can't open file " << netDFileName << ".\n";
        exit(1);
    }

    using node_t = uint32_t;

    char t;
    uint32_t numPins;
    uint32_t numNets;
    uint32_t numModules;
    index_t padOffset;

    netD >> t;  // eat 1st 0
    netD >> numPins >> numNets >> numModules >> padOffset;

    // using Edge = std::pair<int, int>;

    const auto num_vertices = numModules + numNets;
    // const auto R = py::range<node_t>(0, num_vertices);
    auto g = graph_t(num_vertices);

    constexpr index_t bufferSize = 100;
    char lineBuffer[bufferSize];  // Does it work for other compiler?
    netD.getline(lineBuffer, bufferSize);

    node_t w;
    index_t e = numModules - 1;
    char c;
    uint32_t i = 0;
    for (; i < numPins; ++i) {
        if (netD.eof()) {
            std::cerr << "Warning: Unexpected end of file.\n";
            break;
        }
        do {
            netD.get(c);
        } while ((isspace(c) != 0));
        if (c == '\n') {
            continue;
        }
        if (c == 'a') {
            netD >> w;
        } else if (c == 'p') {
            netD >> w;
            w += padOffset;
        }
        do {
            netD.get(c);
        } while ((isspace(c) != 0));
        if (c == 's') {
            ++e;
        }

        // edge_array[i] = Edge(w, e);
        g.add_edge(w, e);

        do {
            netD.get(c);
        } while ((isspace(c) != 0) && c != '\n');
        // switch (c) {
        // case 'O': aPin.setDirection(Pin::OUTPUT); break;
        // case 'I': aPin.setDirection(Pin::INPUT); break;
        // case 'B': aPin.setDirection(Pin::BIDIR); break;
        // }
        if (c != '\n') {
            netD.getline(lineBuffer, bufferSize);
        }
    }

    e -= numModules - 1;
    if (e < numNets) {
        std::cerr << "Warning: number of nets is not " << numNets << ".\n";
        numNets = e;
    } else if (e > numNets) {
        std::cerr << "Error: number of nets is not " << numNets << ".\n";
        exit(1);
    }
    if (i < numPins) {
        std::cerr << "Error: number of pins is not " << numPins << ".\n";
        exit(1);
    }

    // using IndexMap =
    //     typename boost::property_map<graph_t, boost::vertex_index_t>::type;
    // auto index = boost::get(boost::vertex_index, g);
    // auto G = py::grAdaptor<graph_t>{std::move(g)};
    auto H = SimpleNetlist{std::move(g), numModules, numNets};
    H.num_pads = numModules - padOffset - 1;
    return H;
}

// Read the IBM .are format
void readAre(SimpleNetlist& H, std::string_view areFileName) {
    auto are = ifstream{areFileName.data()};
    if (are.fail()) {
        std::cerr << " Could not open " << areFileName << std::endl;
        exit(1);
    }

    using node_t = uint32_t;
    constexpr index_t bufferSize = 100;
    char lineBuffer[bufferSize];

    char c;
    node_t w;
    unsigned int weight;
    // auto totalWeight = 0;
    // xxx index_t smallestWeight = UINT_MAX;
    auto numModules = H.number_of_modules();
    auto padOffset = numModules - H.num_pads - 1;
    auto module_weight = std::vector<unsigned int>(numModules);

    size_t lineno = 1;
    for (size_t i = 0; i < numModules; i++) {
        if (are.eof()) {
            break;
        }
        do {
            are.get(c);
        } while ((isspace(c) != 0));
        if (c == '\n') {
            lineno++;
            continue;
        }
        if (c == 'a') {
            are >> w;
        } else if (c == 'p') {
            are >> w;
            w += node_t(padOffset);
        } else {
            std::cerr << "Syntax error in line " << lineno << ":"
                      << R"(expect keyword "a" or "p")" << std::endl;
            exit(0);
        }

        do {
            are.get(c);
        } while ((isspace(c) != 0));
        if (isdigit(c) != 0) {
            are.putback(c);
            are >> weight;
            module_weight[w] = weight;
        }
        are.getline(lineBuffer, bufferSize);
        lineno++;
    }

    H.module_weight = std::move(module_weight);
}
