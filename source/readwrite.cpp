#include <cctype>                      // for isspace, isdigit
#include <ckpttn/netlist.hpp>          // for SimpleNetlist, index_t, Netlist
#include <cstdint>                     // for uint32_t
#include <cstdlib>                     // for exit, size_t
#include <fstream>                     // for operator<<, basic_ostream, cha...
#include <iostream>                    // for cerr
#include <py2cpp/range.hpp>            // for _iterator
#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph
// #include <py2cpp/py2cpp.hpp>
// #include <__config>      // for std
// #include <__hash_table>  // for __hash_const_iterator, operator!=
#include <boost/utility/string_view.hpp>  // for boost::string_view
#include <type_traits>                    // for move
#include <vector>                         // for vector

// using graph_t =
//     boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
// using node_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
// using edge_t = typename boost::graph_traits<graph_t>::edge_iterator;

using namespace std;

// Read the IBM .netD/.net format. Precondition: Netlist is empty.
void writeJSON(boost::string_view jsonFileName, const SimpleNetlist &hyprgraph) {
    auto json = ofstream{jsonFileName.data()};
    if (json.fail()) {
        cerr << "Error: Can't open file " << jsonFileName << ".\n";
        exit(1);
    }
    json << R"({
 "directed": false,
 "multigraph": false,
 "graph": {
)";

    json << R"( "num_modules": )" << hyprgraph.number_of_modules() << ",\n";
    json << R"( "num_nets": )" << hyprgraph.number_of_nets() << ",\n";
    json << R"( "num_pads": )" << hyprgraph.num_pads << "\n";
    json << " },\n";

    json << R"( "nodes": [)"
         << "\n";
    for (const auto &node : hyprgraph.gr) {
        json << "  { \"id\": " << node << " },\n";
    }
    json << " ],\n";

    json << R"( "links": [)"
         << "\n";
    for (const auto &v : hyprgraph) {
        for (const auto &net : hyprgraph.gr[v]) {
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
auto readNetD(boost::string_view netDFileName) -> SimpleNetlist {
    auto netD = ifstream{netDFileName.data()};
    if (netD.fail()) {
        cerr << "Error: Can't open file " << netDFileName << ".\n";
        exit(1);
    }

    using node_t = uint32_t;

    char t = 0;
    uint32_t numPins = 0;
    uint32_t numNets = 0;
    uint32_t numModules = 0;
    index_t padOffset = 0;

    netD >> t;  // eat 1st 0
    netD >> numPins >> numNets >> numModules >> padOffset;

    // using Edge = pair<int, int>;

    const auto num_vertices = numModules + numNets;
    // const auto R = py::range<node_t>(0, num_vertices);
    auto g = graph_t(num_vertices);

    constexpr index_t bufferSize = 100;
    char lineBuffer[bufferSize];  // Does it work for other compiler?
    netD.getline(lineBuffer, bufferSize);

    node_t w = 0;
    index_t e = numModules - 1;
    char c = 0;
    uint32_t i = 0;
    for (; i < numPins; ++i) {
        if (netD.eof()) {
            cerr << "Warning: Unexpected end of file.\n";
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
        cerr << "Warning: number of nets is not " << numNets << ".\n";
        numNets = e;
    } else if (e > numNets) {
        cerr << "Error: number of nets is not " << numNets << ".\n";
        exit(1);
    }
    if (i < numPins) {
        cerr << "Error: number of pins is not " << numPins << ".\n";
        exit(1);
    }

    // using IndexMap =
    //     typename boost::property_map<graph_t, boost::vertex_index_t>::type;
    // auto index = boost::get(boost::vertex_index, g);
    // auto gr = py::GraphAdaptor<graph_t>{std::move(g)};
    auto hyprgraph = SimpleNetlist{std::move(g), numModules, numNets};
    hyprgraph.num_pads = numModules - padOffset - 1;
    return hyprgraph;
}

// Read the IBM .are format
void readAre(SimpleNetlist &hyprgraph, boost::string_view areFileName) {
    auto are = ifstream{areFileName.data()};
    if (are.fail()) {
        cerr << " Could not open " << areFileName << endl;
        exit(1);
    }

    using node_t = uint32_t;
    constexpr index_t bufferSize = 100;
    char lineBuffer[bufferSize];

    char c = 0;
    node_t w = 0;
    unsigned int weight = 0;
    // auto totalWeight = 0;
    // xxx index_t smallestWeight = UINT_MAX;
    auto numModules = hyprgraph.number_of_modules();
    auto padOffset = numModules - hyprgraph.num_pads - 1;
    auto module_weight = vector<unsigned int>(numModules);

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
            cerr << "Syntax error in line " << lineno << ":"
                 << R"(expect keyword "a" or "p")" << endl;
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

    hyprgraph.module_weight = std::move(module_weight);
}
