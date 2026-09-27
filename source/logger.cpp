/**
 * @file logger.cpp
 * @brief Implementation of logging utilities using spdlog
 *
 * This file implements the logging functionality for the ckpttn library
 * using the spdlog library as the backend.
 */

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <ckpttn/logger.hpp>
#include <memory>

namespace ckpttn {

    /// @brief Create and configure the file logger
    /// @details Extracted into a noexcept helper so the one-time static
    ///          initialization in log_with_spdlog cannot throw.
    /// @return The configured file logger
    auto make_file_logger() noexcept -> std::shared_ptr<spdlog::logger> {
        auto l = spdlog::basic_logger_mt("file_logger", "ckpttn.log");
        spdlog::set_default_logger(l);
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
        return l;
    }

    /**
     * @brief Log a message using spdlog to a file
     *
     * This function creates a file logger that writes to "ckpttn.log",
     * sets it as the default logger, and logs the provided message with
     * an info level. The logger is configured to flush on info level
     * messages to ensure they are written to disk immediately.
     *
     * @param[in] message The message to log
     */
    void log_with_spdlog(const std::string& message) {
        // One-time initialization of file logger
        static const auto logger = make_file_logger();

        spdlog::info("CkPttn message: {}", message);
    }

}  // namespace ckpttn
