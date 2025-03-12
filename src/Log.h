#pragma once

#include <array>
#include <string>

/**
 * @brief Represents different levels of logging used for categorizing log messages.
 */
enum class LogLevel {
    DEBUG,
    INFO,
    ERROR,
    WARNING
};

static inline std::array<std::string, static_cast<size_t>(LogLevel::WARNING) + 1> log_level_names = {
    "DEBUG", "INFO ", "ERROR", "WARNING"
};

class Logger {
public:
    static void log(LogLevel level, const std::string &name, const std::string &message) {
        std::cout << log_level_names[static_cast<size_t>(level)] << ' ' << name << " - " << message << '\n';
    }
};
