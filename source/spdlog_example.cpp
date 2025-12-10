#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>
#include <cstdint>

namespace spd = spdlog;

// Simple function to demonstrate spdlog usage in the ckpttn library
void run_spdlog_example() {
    // Create a file sink for logging
    auto file_sink = std::make_shared<spd::sinks::basic_file_sink_mt>("ckpttn_example.log", true);
    
    // Create a logger with the file sink
    auto logger = std::make_shared<spd::logger>("ckpttn_logger", file_sink);
    
    // Set logging level
    logger->set_level(spd::level::info);
    
    // Set pattern for log messages
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
    
    // Example logging messages
    logger->info("Starting ckpttn spdlog example");
    logger->info("This is a sample logging function for the circuit partitioning library");
    logger->warn("Sample warning message");
    
    // Log some sample data related to partitioning
    std::vector<std::uint8_t> partition_data = {0, 1, 0, 1, 0, 1};
    logger->info("Sample partition data: {} nodes partitioned", partition_data.size());
    
    for (size_t i = 0; i < partition_data.size(); ++i) {
        logger->debug("Node {} assigned to partition {}", i, partition_data[i]);
    }
    
    logger->info("Finished ckpttn spdlog example");
}