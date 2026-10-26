#include "ExpressionParser.h"
#include "Logger.h"
#include "PluginManager.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
constexpr const char* kDefaultPluginDir = "plugins";
}

int main(int argc, char** argv) {
    try {
        std::string pluginDir = kDefaultPluginDir;
        bool quiet = false;
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--quiet") quiet = true;
            else if (a == "--plugins" && i + 1 < argc) pluginDir = argv[++i];
        }
        if (quiet) Logger::setVerbose(false);

        namespace fs = std::filesystem;
        Logger::info("CWD: " + fs::current_path().string());
        Logger::info("Plugin dir: " +
                     fs::absolute(pluginDir).lexically_normal().string());

        PluginManager plugins;
        plugins.loadDirectory(pluginDir);

        auto names = plugins.loadedNames();
        if (names.empty()) {
            Logger::warn("No plugin functions available. "
                         "Only +,-,*,/,^ and parentheses are supported.");
        } else {
            std::string list;
            for (auto& n : names) {
                if (!list.empty()) list += ", ";
                list += n;
            }
            Logger::info("Available functions: " + list);
        }

        std::string line;
        while (true) {
            std::cout << "> " << std::flush;
            if (!std::getline(std::cin, line)) break;
            if (line == "quit" || line == "exit") break;
            if (line.empty()) continue;

            try {
                ExpressionParser parser(plugins);
                std::cout << parser.evaluate(line) << '\n';
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << '\n';
            } catch (...) {
                std::cerr << "Error: unknown exception\n";
            }
        }
        return 0;
    } catch (const std::exception& e) {
        Logger::error(std::string("Fatal: ") + e.what());
        return 1;
    } catch (...) {
        Logger::error("Fatal: unknown exception");
        return 1;
    }
}