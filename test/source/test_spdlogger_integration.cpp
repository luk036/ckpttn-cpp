#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <ckpttn/logger.hpp>
#include <ckpttn/spdlog_example.hpp>
#include <fstream>
#include <iostream>

#include "test_common.hpp"

TEST_CASE("Test spdlogger wrapper function") {
    std::cout << "\n=== Testing spdlogger wrapper function ===" << std::endl;

    // Test 1: Basic logging
    std::cout << "[Test 1] Testing basic logging with wrapper..." << std::endl;
    ckpttn::log_with_spdlog("Test message 1 from wrapper");
    ckpttn::log_with_spdlog("Test message 2 from wrapper");
    ckpttn::log_with_spdlog("Test message 3 from wrapper");
    std::cout << "  [OK] Wrapper function calls completed" << std::endl;

    // Test 2: Verify log file creation
    std::cout << "\n[Test 2] Verifying log file creation..." << std::endl;
    std::ifstream log_file("ckpttn.log");
    if (log_file.is_open()) {
        std::string line;
        int line_count = 0;
        while (std::getline(log_file, line)) {
            line_count++;
        }
        log_file.close();
        std::cout << "  [OK] Log file exists with " << line_count << " lines" << std::endl;
        REQUIRE(line_count >= 3);
    } else {
        std::cout << "  [X] Log file not found" << std::endl;
        REQUIRE(false);
    }
}

TEST_CASE("Test direct spdlog usage") {
    std::cout << "\n=== Testing direct spdlog usage ===" << std::endl;

    // Test 1: Create logger and log messages
    std::cout << "[Test 1] Creating direct logger..." << std::endl;
    try {
        spdlog::drop("direct_logger");
    } catch (...) {
        // Ignore if logger doesn't exist
    }

    auto logger = spdlog::basic_logger_mt("direct_logger", "direct_test.log");
    logger->set_level(spdlog::level::info);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    logger->flush_on(spdlog::level::info);

    std::cout << "  [OK] Logger created successfully" << std::endl;

    // Test 2: Log messages
    std::cout << "\n[Test 2] Logging messages..." << std::endl;
    logger->info("Direct test message 1");
    logger->info("Direct test message 2");
    logger->warn("Direct warning message");
    logger->flush();
    std::cout << "  [OK] Messages logged successfully" << std::endl;

    // Test 3: Verify log file
    std::cout << "\n[Test 3] Verifying log file..." << std::endl;
    std::ifstream log_file("direct_test.log");
    if (log_file.is_open()) {
        std::string line;
        int line_count = 0;
        while (std::getline(log_file, line)) {
            line_count++;
            std::cout << "    Line " << line_count << ": " << line << std::endl;
        }
        log_file.close();
        std::cout << "  [OK] Direct log file exists with " << line_count << " lines" << std::endl;
        REQUIRE(line_count >= 3);
    } else {
        std::cout << "  [X] Direct log file not found" << std::endl;
        REQUIRE(false);
    }
}

TEST_CASE("Test spdlog example function") {
    std::cout << "\n=== Testing spdlog example function ===" << std::endl;

    // Test 1: Run the example function
    std::cout << "[Test 1] Running run_spdlog_example()..." << std::endl;
    run_spdlog_example();
    std::cout << "  [OK] Example function completed" << std::endl;

    // Test 2: Verify example log file
    std::cout << "\n[Test 2] Verifying example log file..." << std::endl;
    std::ifstream log_file("ckpttn_example.log");
    if (log_file.is_open()) {
        std::string line;
        int line_count = 0;
        while (std::getline(log_file, line)) {
            line_count++;
        }
        log_file.close();
        std::cout << "  [OK] Example log file exists with " << line_count << " lines" << std::endl;
        REQUIRE(line_count >= 5);
    } else {
        std::cout << "  [X] Example log file not found" << std::endl;
        REQUIRE(false);
    }
}

TEST_CASE("Test comprehensive spdlogger integration") {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  CkPttn Spdlogger Integration Test      " << std::endl;
    std::cout << "========================================" << std::endl;

    // Part 1: Test wrapper function
    std::cout << "\n[Part 1] Testing wrapper function..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    ckpttn::log_with_spdlog("Integration test initialization");
    ckpttn::log_with_spdlog("Testing circuit partitioning context");
    ckpttn::log_with_spdlog("Integration test completed successfully");
    std::cout << "[OK] Wrapper function works" << std::endl;

    // Part 2: Test direct spdlog
    std::cout << "\n[Part 2] Testing direct spdlog usage..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    try {
        spdlog::drop("integration_test");
    } catch (...) {
        // Ignore
    }
    auto logger = spdlog::basic_logger_mt("integration_test", "integration_test.log");
    logger->set_level(spdlog::level::info);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    logger->flush_on(spdlog::level::info);
    logger->info("Direct spdlog test message");
    logger->flush();
    std::cout << "[OK] Direct spdlog works" << std::endl;

    // Part 3: Verify all log files
    std::cout << "\n[Part 3] Verifying log files..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int wrapper_lines = 0;
    int direct_lines = 0;

    std::ifstream wrapper_file("ckpttn.log");
    if (wrapper_file.is_open()) {
        std::string line;
        while (std::getline(wrapper_file, line)) {
            wrapper_lines++;
        }
        wrapper_file.close();
std::cout << "[OK] ckpttn.log (wrapper) exists with " << wrapper_lines << " lines"
                  << std::endl;
    }

    std::ifstream direct_file("integration_test.log");
    if (direct_file.is_open()) {
        std::string line;
        while (std::getline(direct_file, line)) {
            direct_lines++;
        }
        direct_file.close();
std::cout << "[OK] integration_test.log (direct) exists with " << direct_lines << " lines"
                  << std::endl;
    }

    // Summary
std::cout << "\n[Summary]" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[OK] Spdlogger integration test completed" << std::endl;
    std::cout << "[OK] Logger wrapper function works" << std::endl;
    std::cout << "[OK] Direct spdlog usage works" << std::endl;
    std::cout << "[OK] Log files created successfully" << std::endl;
    std::cout << "========================================" << std::endl;

    REQUIRE(wrapper_lines >= 3);
    REQUIRE(direct_lines >= 1);
}