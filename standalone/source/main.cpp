#define CKPTTN_VERSION "1.0"

#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <ckpttn/NNPartMgr.hpp>
#include <cstdint>
#include <cxxopts.hpp>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <netlistx/netlist.hpp>
#include <netlistx/readwrite.hpp>
#include <random>
#include <string>
#include <xnetwork/classes/graph.hpp>
#include <xnetwork/thread_pool.hpp>

using graph_t = xnetwork::SimpleGraph;
using index_t = std::uint32_t;
using SimpleNetlist = Netlist<graph_t>;
using node_t = typename SimpleNetlist::node_t;

struct PresetConfig {
    double balance_tolerance;
    std::uint8_t num_parts;
    bool use_recursive;
};

enum class Preset { default_preset, quality, highest_quality, deterministic, large_k };

auto get_preset_config(Preset preset, std::uint8_t k) -> PresetConfig {
    switch (preset) {
        case Preset::default_preset:
            return {0.03, k, true};
        case Preset::quality:
            return {0.01, k, false};
        case Preset::highest_quality:
            return {0.005, k, false};
        case Preset::deterministic:
            return {0.03, k, true};
        case Preset::large_k:
            return {0.03, k, true};
        default:
            return {0.03, k, true};
    }
}

auto run_binary_partition(const SimpleNetlist& hyprgraph, double balance_tol,
                          std::span<std::uint8_t> part) -> int {
    using GainMgr = FMBiGainMgr<SimpleNetlist>;
    using ConstrMgr = FMBiConstrMgr<SimpleNetlist>;
    using PartMgr = FMPartMgr<SimpleNetlist, GainMgr, ConstrMgr>;

    MLPartMgr ml_mgr(balance_tol, 2);
    ml_mgr.run_Partition<SimpleNetlist, PartMgr>(hyprgraph, part);
    return ml_mgr.total_cost;
}

auto run_kway_partition(const SimpleNetlist& hyprgraph, double balance_tol,
                        std::span<std::uint8_t> part, std::uint8_t num_parts) -> int {
    using GainMgr = FMKWayGainMgr<SimpleNetlist>;
    using ConstrMgr = FMKWayConstrMgr<SimpleNetlist>;
    using PartMgr = FMPartMgr<SimpleNetlist, GainMgr, ConstrMgr>;

    MLPartMgr ml_mgr(balance_tol, num_parts);
    ml_mgr.run_Partition<SimpleNetlist, PartMgr>(hyprgraph, part);
    return ml_mgr.total_cost;
}

auto run_nn_binary_partition(const SimpleNetlist& hyprgraph, double balance_tol,
                             std::span<std::uint8_t> part) -> int {
    using GainMgr = FMBiGainMgr<SimpleNetlist>;
    using ConstrMgr = FMBiConstrMgr<SimpleNetlist>;
    using PartMgr = NNPartMgr<SimpleNetlist, GainMgr, ConstrMgr>;

    MLPartMgr ml_mgr(balance_tol, 2);
    ml_mgr.run_Partition<SimpleNetlist, PartMgr>(hyprgraph, part);
    return ml_mgr.total_cost;
}

auto run_nn_kway_partition(const SimpleNetlist& hyprgraph, double balance_tol,
                           std::span<std::uint8_t> part, std::uint8_t num_parts) -> int {
    using GainMgr = FMKWayGainMgr<SimpleNetlist>;
    using ConstrMgr = FMKWayConstrMgr<SimpleNetlist>;
    using PartMgr = NNPartMgr<SimpleNetlist, GainMgr, ConstrMgr>;

    MLPartMgr ml_mgr(balance_tol, num_parts);
    ml_mgr.run_Partition<SimpleNetlist, PartMgr>(hyprgraph, part);
    return ml_mgr.total_cost;
}

template <typename Gen> auto random_init_part(std::span<std::uint8_t> part,
                                              const SimpleNetlist& hyprgraph,
                                              std::uint8_t num_parts, Gen& gen) -> void {
    std::uniform_int_distribution<int> dist(0, num_parts - 1);
    for (index_t i = 0; i < hyprgraph.number_of_modules(); ++i) {
        if (!hyprgraph.module_fixed.contains(i)) {
            part[i] = static_cast<std::uint8_t>(dist(gen));
        }
    }
}

auto main(int argc, char** argv) -> int {
    cxxopts::Options options(
        *argv, "ckpttn - A hypergraph partitioner compatible with hMetis and KaHyPar");

    std::string hypergraph_file;
    std::uint32_t k = 2;
    double epsilon = 0.05;

    std::string input_format_str = "auto";
    std::string fixed_file;

    std::string output_file;
    std::string output_format_str = "hmetis";
    bool quiet = false;

    std::string preset_str = "default";
    std::string objective = "cut";
    std::string mode_str = "recursive";
    std::uint32_t threads = 1;

    std::uint32_t seed = 0;
    bool verbose = false;
    double time_limit = 0.0;
    std::uint32_t max_quality = 0;

    options.add_options()("h,help", "Show help")("v,version", "Print the current version number")

        ("hypergraph_file", "Input hypergraph file", cxxopts::value<std::string>(hypergraph_file))(
            "k", "Number of parts", cxxopts::value<std::uint32_t>(k)->default_value("2"))(
            "epsilon", "Imbalance factor (0.05 = 5%)",
            cxxopts::value<double>(epsilon)->default_value("0.05"))

            ("i,input-format", "Input format: hmetis, json, yosys, dimacs, netd, auto",
             cxxopts::value<std::string>(input_format_str)->default_value("auto"))(
                "f,fixed", "File with pre-assigned vertices",
                cxxopts::value<std::string>(fixed_file)->default_value(""))

                ("o,output", "Output partition file (default: stdout)",
                 cxxopts::value<std::string>(output_file)->default_value(""))(
                    "output-format", "Output format: hmetis, json",
                    cxxopts::value<std::string>(output_format_str)->default_value("hmetis"))(
                    "q,quiet", "Suppress output")

                    ("p,preset",
                     "Preset: default, quality, highest_quality, deterministic, large_k",
                     cxxopts::value<std::string>(preset_str)->default_value("default"))(
                        "objective", "Objective: cut, km1, soed, km1a",
                        cxxopts::value<std::string>(objective)->default_value("cut"))(
                        "mode", "Mode: direct, recursive",
                        cxxopts::value<std::string>(mode_str)->default_value("recursive"))(
                        "t,threads", "Number of starts (multi-start)",
                        cxxopts::value<std::uint32_t>(threads)->default_value("1"))

                        ("s,seed", "Random seed (0 = use random device)",
                         cxxopts::value<std::uint32_t>(seed)->default_value("0"))("verbose",
                                                                                  "Verbose output")

                            ("time-limit", "Time limit in seconds",
                             cxxopts::value<double>(time_limit)->default_value("0"))(
                                "max-quality", "Maximum quality",
                                cxxopts::value<std::uint32_t>(max_quality)->default_value("0"));

    options.parse_positional({"hypergraph_file", "k", "epsilon"});

    auto result = options.parse(argc, argv);

    if (result["help"].as<bool>()) {
        std::cout << options.help() << '\n';
        std::cout << R"(
Usage:
  ckpttn <hypergraph_file> [k] [epsilon]
  ckpttn <hypergraph_file> -k <k> -e <epsilon>

Examples:
  ckpttn circuit.hgr 2 5
  ckpttn circuit.hgr 4 10 -o partition.txt
  ckpttn circuit.hgr -k 4 -e 0.03 -p quality
  ckpttn circuit.hgr 2 5 -f fix.txt
  ckpttn circuit.hgr 2 5 -s 42
  ckpttn circuit.hgr 2 5 --mode direct --verbose
  ckpttn circuit.hgr 2 5 -t 8 -s 42
  ckpttn circuit.json 2 5 -i yosys --verbose

Compatible with hMetis and KaHyPar CLI.
)";
        return 0;
    }

    if (result["version"].as<bool>()) {
        std::cout << "ckpttn " << CKPTTN_VERSION << '\n';
        return 0;
    }

    auto use_yosys = false;
    InputFormat input_format = InputFormat::auto_detect;
    if (input_format_str == "hmetis") {
        input_format = InputFormat::hmetis;
    } else if (input_format_str == "json") {
        input_format = InputFormat::json;
    } else if (input_format_str == "yosys") {
        use_yosys = true;
    } else if (input_format_str == "dimacs") {
        input_format = InputFormat::dimacs;
    } else if (input_format_str == "netd") {
        input_format = InputFormat::netD;
    } else {
        input_format = InputFormat::auto_detect;
    }

    Preset preset;
    if (preset_str == "default") {
        preset = Preset::default_preset;
    } else if (preset_str == "quality") {
        preset = Preset::quality;
    } else if (preset_str == "highest_quality") {
        preset = Preset::highest_quality;
    } else if (preset_str == "deterministic") {
        preset = Preset::deterministic;
    } else if (preset_str == "large_k") {
        preset = Preset::large_k;
    } else {
        preset = Preset::default_preset;
    }

    const auto use_recursive = (mode_str == "recursive");

    OutputFormat output_format;
    if (output_format_str == "json") {
        output_format = OutputFormat::json;
    } else {
        output_format = OutputFormat::hmetis;
    }

    verbose = result["verbose"].as<bool>();
    quiet = result["quiet"].as<bool>();
    if (quiet) {
        verbose = false;
    }

    if (hypergraph_file.empty()) {
        std::cerr << "Error: hypergraph_file is required.\n";
        std::cerr << "Use --help for usage information.\n";
        return 1;
    }

    if (k < 2) {
        std::cerr << "Error: k must be >= 2.\n";
        return 1;
    }

    if (epsilon > 1.0) {
        epsilon = epsilon / 100.0;
    }
    if (epsilon < 0.0 || epsilon > 1.0) {
        std::cerr << "Error: epsilon must be between 0 and 1.\n";
        return 1;
    }

    auto config = get_preset_config(preset, static_cast<std::uint8_t>(k));
    config.balance_tolerance = epsilon;

    if (verbose) {
        std::cerr << "Reading hypergraph from " << hypergraph_file << "...\n";
    }

    auto hyprgraph = use_yosys ? read_yosys_json(hypergraph_file)
                               : read_hypergraph(hypergraph_file, input_format);

    if (!fixed_file.empty()) {
        auto fix_fs = std::ifstream{fixed_file};
        if (fix_fs.fail()) {
            std::cerr << "Error: Can't open fixed modules file " << fixed_file << ".\n";
            return 1;
        }
        std::uint32_t module_id = 0;
        while (fix_fs >> module_id) {
            hyprgraph.module_fixed.insert(module_id);
        }
        hyprgraph.has_fixed_modules = true;
        if (verbose) {
            std::cerr << "Fixed modules: " << hyprgraph.module_fixed.size() << '\n';
        }
    }

    if (verbose) {
        std::cerr << "Hypergraph: " << hyprgraph.number_of_modules() << " vertices, "
                  << hyprgraph.number_of_nets() << " nets\n";
        std::cerr << "K=" << k << ", epsilon=" << epsilon << ", preset=" << preset_str << '\n';
    }

    auto num_modules = hyprgraph.number_of_modules();
    const auto num_starts = std::max(threads, 1U);

    if (verbose) {
        if (seed != 0) {
            std::cerr << "Base seed: " << seed;
        }
        if (num_starts > 1) {
            std::cerr << ", starts: " << num_starts;
        }
        if (seed != 0 || num_starts > 1) {
            std::cerr << '\n';
        }
        std::cerr << "Running partitioning (preset: " << preset_str
                  << ", mode: " << (use_recursive ? "recursive" : "direct") << ")...\n";
    }

    auto best_part = std::vector<std::uint8_t>(num_modules, 0);
    auto best_cost = std::numeric_limits<int>::max();

    if (num_starts == 1) {
        const auto start_seed = seed != 0 ? seed : std::random_device{}();
        auto local_gen = std::mt19937{start_seed};
        random_init_part(best_part, hyprgraph, static_cast<std::uint8_t>(k), local_gen);
        best_cost
            = k == 2
                  ? (use_recursive
                         ? run_binary_partition(hyprgraph, config.balance_tolerance, best_part)
                         : run_nn_binary_partition(hyprgraph, config.balance_tolerance, best_part))
                  : (use_recursive
                         ? run_kway_partition(hyprgraph, config.balance_tolerance, best_part,
                                              static_cast<std::uint8_t>(k))
                         : run_nn_kway_partition(hyprgraph, config.balance_tolerance, best_part,
                                                 static_cast<std::uint8_t>(k)));
    } else {
        xnetwork::thread_pool pool(num_starts);
        std::vector<std::future<std::pair<int, std::vector<std::uint8_t>>>> futures;
        futures.reserve(num_starts);

        for (auto start = 0U; start < num_starts; ++start) {
            const auto start_seed = seed != 0 ? seed + start * 104729U : std::random_device{}();
            futures.emplace_back(pool.enqueue([&hyprgraph, &config, k, use_recursive, start_seed,
                                               num_modules]()
                                                  -> std::pair<int, std::vector<std::uint8_t>> {
                auto local_gen = std::mt19937{start_seed};
                auto local_part = std::vector<std::uint8_t>(num_modules, 0);
                random_init_part(local_part, hyprgraph, static_cast<std::uint8_t>(k), local_gen);
                const auto local_cost
                    = k == 2
                          ? (use_recursive ? run_binary_partition(
                                                 hyprgraph, config.balance_tolerance, local_part)
                                           : run_nn_binary_partition(
                                                 hyprgraph, config.balance_tolerance, local_part))
                          : (use_recursive
                                 ? run_kway_partition(hyprgraph, config.balance_tolerance,
                                                      local_part, static_cast<std::uint8_t>(k))
                                 : run_nn_kway_partition(hyprgraph, config.balance_tolerance,
                                                         local_part, static_cast<std::uint8_t>(k)));
                return {local_cost, std::move(local_part)};
            }));
        }

        for (auto start = 0U; start < num_starts; ++start) {
            auto [local_cost, local_part] = futures[start].get();
            if (verbose) {
                std::cerr << "  Start " << start + 1 << '/' << num_starts << " cost: " << local_cost
                          << '\n';
            }
            if (local_cost < best_cost) {
                best_cost = local_cost;
                best_part = std::move(local_part);
            }
        }
    }
    auto part = std::move(best_part);

    {
        const auto balanced
            = (k == 2) ? FMBiConstrMgr<SimpleNetlist>(hyprgraph, config.balance_tolerance)
                             .final_check(part)
                       : FMKWayConstrMgr<SimpleNetlist>(hyprgraph, config.balance_tolerance,
                                                        static_cast<std::uint8_t>(k))
                             .final_check(part);
        if (!balanced) {
            std::cerr << "Warning: final partition does not satisfy the balance constraint\n";
        }
    }

    if (verbose) {
        std::cerr << "Partitioning cost: " << best_cost << '\n';
        std::cerr << "Partition written to stdout\n";
    }

    if (output_file.empty()) {
        write_partition(part, std::cout, output_format);
    } else {
        auto file = std::ofstream{output_file};
        if (file.fail()) {
            std::cerr << "Error: Can't open output file " << output_file << ".\n";
            return 1;
        }
        write_partition(part, file, output_format);
    }

    return 0;
}