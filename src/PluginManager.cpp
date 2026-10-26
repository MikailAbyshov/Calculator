#include "PluginManager.h"
#include "Logger.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace fs = std::filesystem;

namespace {

void* openLibrary(const std::string& path, std::string& err) {
#ifdef _WIN32
    HMODULE h = ::LoadLibraryA(path.c_str());
    if (!h) {
        err = "LoadLibrary failed, error=" + std::to_string(::GetLastError());
    }
    return reinterpret_cast<void*>(h);
#else
    void* h = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!h) {
        const char* e = ::dlerror();
        err = e ? e : "dlopen failed";
    }
    return h;
#endif
}

void closeLibrary(void* handle) {
    if (!handle) return;
#ifdef _WIN32
    ::FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
    ::dlclose(handle);
#endif
}

void* resolveSymbol(void* handle, const char* name) {
#ifdef _WIN32
    return reinterpret_cast<void*>(
        ::GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
#else
    return ::dlsym(handle, name);
#endif
}

bool isPluginFile(const fs::path& p) {
    if (!p.has_extension()) return false;
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
#ifdef _WIN32
    return ext == ".dll";
#else
    return ext == ".so" || ext == ".dylib";
#endif
}

}

PluginManager::~PluginManager() {
    unloadAll();
}

void PluginManager::unloadAll() {
    registry_.clear();
    for (auto& p : plugins_) {
        closeLibrary(p->handle);
        p->handle = nullptr;
    }
    plugins_.clear();
}

size_t PluginManager::loadDirectory(const std::string& directory) {
    std::error_code ec;
    if (!fs::exists(directory, ec) || !fs::is_directory(directory, ec)) {
        Logger::warn("Plugin directory not found: " + directory);
        return 0;
    }

    size_t loaded = 0;
    for (const auto& entry : fs::directory_iterator(directory, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (!isPluginFile(entry.path())) continue;

        const std::string path = entry.path().string();

        std::string err;
        void* handle = openLibrary(path, err);
        if (!handle) {
            Logger::warn("Failed to load '" + path + "': " + err);
            continue;
        }

        auto getter = reinterpret_cast<GetPluginFunctionFn>(
            resolveSymbol(handle, CALC_PLUGIN_ENTRY));
        if (!getter) {
            Logger::warn("No '" + std::string(CALC_PLUGIN_ENTRY) +
                         "' symbol in '" + path + "'");
            closeLibrary(handle);
            continue;
        }

        const PluginFunction* fn = nullptr;
        try {
            fn = getter();
        } catch (const std::exception& e) {
            Logger::warn("Plugin '" + path + "' threw during init: " + e.what());
            closeLibrary(handle);
            continue;
        } catch (...) {
            Logger::warn("Plugin '" + path + "' threw unknown exception during init");
            closeLibrary(handle);
            continue;
        }

        if (!fn || !fn->name || !fn->call || fn->arity < 0) {
            Logger::warn("Plugin '" + path + "' returned invalid description");
            closeLibrary(handle);
            continue;
        }

        const std::string fname = fn->name;
        if (fname.empty()) {
            Logger::warn("Plugin '" + path + "' has empty function name");
            closeLibrary(handle);
            continue;
        }

        if (registry_.count(fname)) {
            Logger::warn("Duplicate function '" + fname + "' from '" + path + "' — skipped");
            closeLibrary(handle);
            continue;
        }

        auto rec = std::make_unique<LoadedPlugin>();
        rec->filePath = path;
        rec->fn = fn;
        rec->handle = handle;
        registry_[fname] = fn;
        plugins_.push_back(std::move(rec));

        Logger::info("Loaded function '" + fname + "' from " + path);
        ++loaded;
    }

    if (loaded == 0) {
        Logger::warn("No plugins were loaded from: " + directory);
    }
    return loaded;
}

bool PluginManager::has(const std::string& name) const {
    return registry_.count(name) != 0;
}

double PluginManager::call(const std::string& name, double arg) const {
    auto it = registry_.find(name);
    if (it == registry_.end()) {
        throw std::runtime_error("Unknown function: " + name);
    }
    return it->second->call(arg);
}

std::vector<std::string> PluginManager::loadedNames() const {
    std::vector<std::string> names;
    names.reserve(registry_.size());
    for (auto& kv : registry_) names.push_back(kv.first);
    return names;
}