#pragma once

#include <string>

class Logger {
public:
    static void info(const std::string &message);

    static void warn(const std::string &message);

    static void error(const std::string &message);

    static void errorRaw(const char *message) noexcept;

    static void setVerbose(bool verbose);

private:
    static bool verbose_;
};