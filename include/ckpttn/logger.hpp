#pragma once

#include <string>

namespace ckpttn {

/**
 * @brief Log a message using spdlog
 *
 * This function provides a simple wrapper around spdlog for logging messages.
 * It creates a file logger that writes to "ckpttn.log" and logs at the info level.
 *
 * @param message The message to log
 */
void log_with_spdlog(const std::string& message);

} // namespace ckpttn