#include "Logger.h"

#include <iostream>

bool Logger::verbose_ = true;

void Logger::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void Logger::info(const std::string &message) {
    if (verbose_) {
        std::cerr << "[INFO]  " << message << '\n';
    }
}

void Logger::warn(const std::string &message) {
    std::cerr << "[WARN]  " << message << '\n';
}

void Logger::error(const std::string &message) {
    std::cerr << "[ERROR] " << message << '\n';
}

void Logger::errorRaw(const char *message) noexcept {
    std::cerr << "[ERROR] " << message << '\n';
}