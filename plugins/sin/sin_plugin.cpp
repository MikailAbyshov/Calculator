#include "PluginAPI.h"
#include <cmath>
#include <stdexcept>

namespace {
    constexpr double kPi = 3.14159265358979323846;

    double sinDeg(double x) {
        if (!std::isfinite(x)) throw std::runtime_error("sin: argument is not finite");
        return std::sin(x * kPi / 180.0);
    }

    const PluginFunction kFn{
        "sin",
        1,
        &sinDeg
    };
}

CALC_EXPORT const PluginFunction* CalcGetPluginFunction() {
    return &kFn;
}