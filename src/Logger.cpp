#include "Logger.h"
#include <iostream>

bool Logger::verbose_ = true;

void Logger::setVerbose(bool v) { verbose_ = v; }

void Logger::info(const std::string& msg) {
    if (verbose_) std::cerr << "[INFO]  " << msg << '\n';
}
void Logger::warn(const std::string& msg) {
    std::cerr << "[WARN]  " << msg << '\n';
}
void Logger::error(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << '\n';
}