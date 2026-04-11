#pragma once

#include <cstdint>
#include <fstream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include <netlistx/netlist.hpp>
#include <xnetwork/classes/graph.hpp>

enum class InputFormat { hmetis, json, dimacs, netD, auto_detect };

enum class OutputFormat { hmetis, json };

auto detect_input_format(const std::string& filename) -> InputFormat;

auto read_hmetis_format(const std::string& filename) -> SimpleNetlist;

auto read_json_format(const std::string& filename) -> SimpleNetlist;

auto read_dimacs_format(const std::string& filename) -> SimpleNetlist;

auto read_netD_format(const std::string& filename) -> SimpleNetlist;

auto read_hypergraph(const std::string& filename,
                    InputFormat format = InputFormat::auto_detect) -> SimpleNetlist;

void write_hmetis_partition(const std::vector<std::uint8_t>& part, std::ostream& os);

void write_json_partition(const std::vector<std::uint8_t>& part, std::ostream& os);

void write_partition(const std::vector<std::uint8_t>& part, std::ostream& os,
                  OutputFormat format = OutputFormat::hmetis);