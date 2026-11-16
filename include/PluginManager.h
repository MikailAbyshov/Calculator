#pragma once

#include "PluginAPI.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class PluginManager {
public:
    PluginManager() = default;

    ~PluginManager();

    PluginManager(const PluginManager &) = delete;

    PluginManager &operator=(const PluginManager &) = delete;

    std::size_t loadDirectory(const std::string &directory);

    [[nodiscard]] bool has(const std::string &name) const;

    [[nodiscard]] double call(const std::string &name, double arg) const;

    [[nodiscard]] std::vector<std::string> loadedNames() const;

private:
    struct LoadedPlugin {
        std::string filePath;
        const PluginFunction *function = nullptr;
        void *handle = nullptr;
    };

    void unloadAll();

    std::vector<std::unique_ptr<LoadedPlugin>> plugins_;
    std::map<std::string, const PluginFunction *> registry_;
};