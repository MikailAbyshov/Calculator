#include "ExpressionParser.h"
#include "Logger.h"
#include "PluginManager.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr const char *kDefaultPluginDir = "plugins";

struct Options {
    std::string pluginDirectory = kDefaultPluginDir;
    bool quiet = false;
};

Options parseArguments(int argc, char **argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        std::string argument = argv[index];
        if (argument == "--quiet") {
            options.quiet = true;
        } else if (argument == "--plugins" && index + 1 < argc) {
            options.pluginDirectory = argv[++index];
        }
    }
    return options;
}

void printAvailableFunctions(const PluginManager &plugins) {
    auto names = plugins.loadedNames();
    if (names.empty()) {
        Logger::warn("No plugin functions available. "
                     "Only +,-,*,/,^ and parentheses are supported.");
        return;
    }
    std::string list;
    for (const auto &name : names) {
        if (!list.empty()) {
            list += ", ";
        }
        list += name;
    }
    Logger::info("Available functions: " + list);
}

void runRepl(const PluginManager &plugins) {
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line == "quit" || line == "exit") {
            break;
        }
        if (line.empty()) {
            continue;
        }
        try {
            ExpressionParser parser(plugins);
            std::cout << parser.evaluate(line) << '\n';
        } catch (const std::exception &exception) {
            std::cerr << "Error: " << exception.what() << '\n';
        } catch (...) {
            std::cerr << "Error: unknown exception\n";
        }
    }
}

}  // namespace

int main(int argc, char **argv) {
    try {
        Options options = parseArguments(argc, argv);
        if (options.quiet) {
            Logger::setVerbose(false);
        }

        namespace fs = std::filesystem;
        Logger::info("CWD: " + fs::current_path().string());
        Logger::info("Plugin dir: " +
                     fs::absolute(options.pluginDirectory)
                         .lexically_normal()
                         .string());

        PluginManager plugins;
        plugins.loadDirectory(options.pluginDirectory);
        printAvailableFunctions(plugins);
        runRepl(plugins);
        return 0;
    } catch (const std::exception &exception) {
        Logger::errorRaw(exception.what());
        return 1;
    } catch (...) {
        Logger::errorRaw("Fatal: unknown exception");
        return 1;
    }
}