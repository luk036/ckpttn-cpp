#include <cctype>  // for isspace, isdigit
#include <ckpttn/readwrite.hpp>
#include <cstdint>                     // for uint32_t
#include <cstdlib>                     // for exit, size_t
#include <fstream>                     // for operator<<, basic_ostream, cha...
#include <iostream>                    // for cerr
#include <netlistx/netlist.hpp>        // for SimpleNetlist, index_t, Netlist
#include <py2cpp/range.hpp>            // for _iterator
#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph
// #include <py2cpp/py2cpp.hpp>
// #include <__config>      // for std
// #include <__hash_table>  // for __hash_const_iterator, operator!=
#include <sstream>      // for istringstream, ostringstream
#include <string_view>  // for std::string_view
#include <vector>       // for vector

// using graph_t =
//     boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
// using node_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
// using edge_t = typename boost::graph_traits<graph_t>::edge_iterator;

using namespace std;

// Read the IBM .netD/.net format. Precondition: Netlist is empty.
void writeJSON(std::string_view jsonFileName, const SimpleNetlist& hyprgraph) {
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
    for (const auto& node : hyprgraph.gr) {
        json << "  { \"id\": " << node << " },\n";
    }
    json << " ],\n";

    json << R"( "links": [)"
         << "\n";
    for (const auto& v : hyprgraph) {
        for (const auto& net : hyprgraph.gr[v]) {
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
void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName) {
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

// ============================================================================
// New read/write functions for CLI
// ============================================================================

auto detect_input_format(const std::string& filename) -> InputFormat {
    auto n = filename.size();
    if (n >= 4 && filename.substr(n - 4) == ".net") {
        return InputFormat::netD;
    }
    if (n >= 4 && filename.substr(n - 4) == ".hgr") {
        return InputFormat::hmetis;
    }
    if (n >= 5 && filename.substr(n - 5) == ".json") {
        return InputFormat::json;
    }
    if (n >= 6 && filename.substr(n - 6) == ".graph") {
        return InputFormat::hmetis;
    }
    if (n >= 7 && filename.substr(n - 7) == ".dimacs") {
        return InputFormat::dimacs;
    }
    return InputFormat::auto_detect;
}

auto read_hmetis_format(const std::string& filename) -> SimpleNetlist {
    auto file = ifstream{filename};
    if (file.fail()) {
        cerr << "Error: Can't open file " << filename << ".\n";
        exit(1);
    }

    uint32_t num_nets = 0;
    uint32_t num_vertices = 0;
    uint32_t fmt = 0;
    file >> num_nets >> num_vertices;
    if (file.fail()) {
        cerr << "Error: Invalid hMetis format in file " << filename << ".\n";
        exit(1);
    }
    file >> fmt;

    const auto num_modules = num_vertices;
    const auto total_vertices = num_modules + num_nets;
    auto g = graph_t(total_vertices);

    string line;
    getline(file, line);

    uint32_t net_idx = 0;
    for (; net_idx < num_nets && getline(file, line); ++net_idx) {
        if (line.empty() || line[0] == 'c') {
            --net_idx;
            continue;
        }
        istringstream iss(line);
        uint32_t v;
        while (iss >> v) {
            if (v < num_modules) {
                g.add_edge(v, num_modules + net_idx);
            }
        }
    }

    return SimpleNetlist{g, num_modules, num_nets};
}

auto read_json_format(const std::string& filename) -> SimpleNetlist {
    auto file = ifstream{filename};
    if (file.fail()) {
        cerr << "Error: Can't open file " << filename << ".\n";
        exit(1);
    }

    ostringstream oss;
    oss << file.rdbuf();

    cerr << "Error: JSON format not fully implemented.\n";
    exit(1);
}

auto read_dimacs_format(const std::string& filename) -> SimpleNetlist {
    auto file = ifstream{filename};
    if (file.fail()) {
        cerr << "Error: Can't open file " << filename << ".\n";
        exit(1);
    }

    uint32_t num_vertices = 0;
    uint32_t num_nets = 0;
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (line[0] == 'c') {
            continue;
        }

        if (line[0] == 'p') {
            istringstream iss(line);
            string p, hypre;
            iss >> p >> hypre >> num_vertices >> num_nets;
            continue;
        }

        if (line[0] == 'e') {
            continue;
        }
    }

    auto g = graph_t(num_vertices + num_nets);
    return SimpleNetlist{g, num_vertices, num_nets};
}

auto read_netD_format(const std::string& filename) -> SimpleNetlist {
    auto netD = ifstream{filename};
    if (netD.fail()) {
        cerr << "Error: Can't open file " << filename << ".\n";
        exit(1);
    }

    char t = 0;
    uint32_t numPins = 0;
    uint32_t numNets = 0;
    uint32_t numModules = 0;
    index_t padOffset = 0;

    netD >> t;
    netD >> numPins >> numNets >> numModules >> padOffset;

    const auto num_vertices = numModules + numNets;
    auto g = graph_t(num_vertices);

    constexpr index_t bufferSize = 100;
    char lineBuffer[bufferSize];
    netD.getline(lineBuffer, bufferSize);

    index_t w = 0;
    index_t e = numModules - 1;
    char c = 0;
    uint32_t i = 0;
    for (; i < numPins; ++i) {
        if (netD.eof()) {
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

        g.add_edge(w, e);

        do {
            netD.get(c);
        } while ((isspace(c) != 0) && c != '\n');
        if (c != '\n') {
            netD.getline(lineBuffer, bufferSize);
        }
    }

    e -= numModules - 1;
    if (e < numNets) {
        numNets = e;
    }

    auto hyprgraph = SimpleNetlist{g, numModules, numNets};
    hyprgraph.num_pads = numModules - padOffset - 1;
    return hyprgraph;
}

auto read_hypergraph(const std::string& filename, InputFormat format) -> SimpleNetlist {
    auto actual_format = format;
    if (format == InputFormat::auto_detect) {
        actual_format = detect_input_format(filename);
    }

    switch (actual_format) {
        case InputFormat::hmetis:
            return read_hmetis_format(filename);
        case InputFormat::json:
            return read_json_format(filename);
        case InputFormat::dimacs:
            return read_dimacs_format(filename);
        case InputFormat::netD:
            return read_netD_format(filename);
        case InputFormat::auto_detect:
            return read_netD_format(filename);
        default:
            cerr << "Error: Unknown input format.\n";
            exit(1);
    }
}

void write_hmetis_partition(const vector<uint8_t>& part, ostream& os) {
    for (const auto p : part) {
        os << static_cast<int>(p) << "\n";
    }
}

void write_json_partition(const vector<uint8_t>& part, ostream& os) {
    os << "[";
    for (size_t i = 0; i < part.size(); ++i) {
        os << static_cast<int>(part[i]);
        if (i < part.size() - 1) {
            os << ", ";
        }
    }
    os << "]\n";
}

void write_partition(const vector<uint8_t>& part, ostream& os, OutputFormat format) {
    if (format == OutputFormat::json) {
        write_json_partition(part, os);
    } else {
        write_hmetis_partition(part, os);
    }
}
