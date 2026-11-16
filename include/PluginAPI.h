#pragma once

#ifdef _WIN32
#  define CALC_EXPORT extern "C" __declspec(dllexport)
#else
#  define CALC_EXPORT extern "C" __attribute__((visibility("default")))
#endif

struct PluginFunction {
    const char* name;
    int         arity;
    double    (*call)(double);
};

using GetPluginFunctionFn = const PluginFunction* (*)();

#define CALC_PLUGIN_ENTRY "CalcGetPluginFunction"