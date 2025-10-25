#pragma once

#include <iostream>
#include <string>
#include <cstdlib>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};

class Logger {
public:
    static LogLevel get_level() {
        static LogLevel cached_level = []() {
            const char* env_log_level = std::getenv("LOG_LEVEL");
            if (!env_log_level) {
                return LogLevel::INFO;
            }

            std::string level_str(env_log_level);
            if (level_str == "DEBUG") return LogLevel::DEBUG;
            if (level_str == "INFO") return LogLevel::INFO;
            if (level_str == "WARN") return LogLevel::WARN;
            if (level_str == "ERROR") return LogLevel::ERROR;
            if (level_str == "NONE") return LogLevel::NONE;

            return LogLevel::INFO;
        }();
        return cached_level;
    }

    static void trace(const std::string& message) {
        if (get_level() <= LogLevel::DEBUG) {
            std::cout << "[TRACE] " << message << std::endl;
        }
    }

    static void debug(const std::string& message) {
        if (get_level() <= LogLevel::DEBUG) {
            std::cout << "[DEBUG] " << message << std::endl;
        }
    }

    static void info(const std::string& message) {
        if (get_level() <= LogLevel::INFO) {
            std::cout << "[INFO] " << message << std::endl;
        }
    }

    static void warn(const std::string& message) {
        if (get_level() <= LogLevel::WARN) {
            std::cerr << "[WARN] " << message << std::endl;
        }
    }

    static void error(const std::string& message) {
        if (get_level() <= LogLevel::ERROR) {
            std::cerr << "[ERROR] " << message << std::endl;
        }
    }
};
