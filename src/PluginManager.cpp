#include "PluginManager.h"

#include "Logger.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace fs = std::filesystem;

namespace {

void *openLibrary(const std::string &path, std::string &errorMessage) {
#ifdef _WIN32
    HMODULE module = ::LoadLibraryA(path.c_str());
    if (module == nullptr) {
        errorMessage = "LoadLibrary failed, error=" +
                       std::to_string(::GetLastError());
    }
    return reinterpret_cast<void *>(module);
#else
    void *module = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
        const char *error = ::dlerror();
        if (error != nullptr) {
            errorMessage = error;
        } else {
            errorMessage = "dlopen failed";
        }
    }
    return module;
#endif
}

void closeLibrary(void *handle) {
    if (handle == nullptr) {
        return;
    }
#ifdef _WIN32
    ::FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
    ::dlclose(handle);
#endif
}

void *resolveSymbol(void *handle, const char *name) {
#ifdef _WIN32
    return reinterpret_cast<void *>(
        ::GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
#else
    return ::dlsym(handle, name);
#endif
}

bool isPluginFile(const fs::path &path) {
    if (!path.has_extension()) {
        return false;
    }
    std::string extension = path.extension().string();
    std::transform(
        extension.begin(), extension.end(), extension.begin(),
        [](unsigned char symbol) {
            return static_cast<char>(std::tolower(symbol));
        });
#ifdef _WIN32
    return extension == ".dll";
#else
    return extension == ".so" || extension == ".dylib";
#endif
}

std::string joinError(const std::string &prefix,
                      const std::string &detail) {
    std::string message = prefix;
    message += detail;
    return message;
}

}  // namespace

PluginManager::~PluginManager() {
    unloadAll();
}

void PluginManager::unloadAll() {
    registry_.clear();
    for (const auto &plugin : plugins_) {
        closeLibrary(plugin->handle);
        plugin->handle = nullptr;
    }
    plugins_.clear();
}

std::size_t PluginManager::loadDirectory(const std::string &directory) {
    std::error_code errorCode;
    if (!fs::exists(directory, errorCode) ||
        !fs::is_directory(directory, errorCode)) {
        Logger::warn("Plugin directory not found: " + directory);
        return 0;
    }

    std::size_t loaded = 0;
    for (const auto &entry : fs::directory_iterator(directory, errorCode)) {
        if (errorCode) {
            break;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        if (!isPluginFile(entry.path())) {
            continue;
        }

        const std::string path = entry.path().string();

        std::string errorMessage;
        void *handle = openLibrary(path, errorMessage);
        if (handle == nullptr) {
            Logger::warn(joinError("Failed to load '" + path + "': ",
                                   errorMessage));
            continue;
        }

        auto getter = reinterpret_cast<GetPluginFunctionFn>(
            resolveSymbol(handle, CALC_PLUGIN_ENTRY));
        if (getter == nullptr) {
            Logger::warn(joinError("No '" +
                                       std::string(CALC_PLUGIN_ENTRY) +
                                       "' symbol in '",
                                   path + "'"));
            closeLibrary(handle);
            continue;
        }

        const PluginFunction *function = nullptr;
        try {
            function = getter();
        } catch (const std::exception &exception) {
            Logger::warn(joinError("Plugin '" + path + "' threw during init: ",
                                   exception.what()));
            closeLibrary(handle);
            continue;
        } catch (...) {
            Logger::warn(
                "Plugin '" + path + "' threw unknown exception during init");
            closeLibrary(handle);
            continue;
        }

        if (function == nullptr || function->name == nullptr ||
            function->call == nullptr || function->arity < 0) {
            Logger::warn("Plugin '" + path + "' returned invalid description");
            closeLibrary(handle);
            continue;
        }

        const std::string functionName = function->name;
        if (functionName.empty()) {
            Logger::warn("Plugin '" + path + "' has empty function name");
            closeLibrary(handle);
            continue;
        }

        if (registry_.count(functionName) != 0) {
            std::string message = "Duplicate function '";
            message += functionName;
            message += "' from '";
            message += path;
            message += "' — skipped";
            Logger::warn(message);
            closeLibrary(handle);
            continue;
        }

        auto record = std::make_unique<LoadedPlugin>();
        record->filePath = path;
        record->function = function;
        record->handle = handle;
        registry_[functionName] = function;
        plugins_.push_back(std::move(record));

        std::string message = "Loaded function '";
        message += functionName;
        message += "' from ";
        message += path;
        Logger::info(message);
        ++loaded;
    }

    if (loaded == 0) {
        Logger::warn("No plugins were loaded from: " + directory);
    }
    return loaded;
}

bool PluginManager::has(const std::string &name) const {
    return registry_.count(name) != 0;
}

double PluginManager::call(const std::string &name, double arg) const {
    auto iter = registry_.find(name);
    if (iter == registry_.end()) {
        throw std::runtime_error("Unknown function: " + name);
    }
    return iter->second->call(arg);
}

std::vector<std::string> PluginManager::loadedNames() const {
    std::vector<std::string> names;
    names.reserve(registry_.size());
    for (const auto &entry : registry_) {
        names.push_back(entry.first);
    }
    return names;
}