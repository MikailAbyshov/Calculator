#include "PluginAPI.h"
#include <cmath>
#include <stdexcept>

namespace {
    constexpr double kPi = 3.14159265358979323846;

    double deg(double x) {
        if (!std::isfinite(x)) throw std::runtime_error("deg: argument is not finite");
        return x * 180.0 / kPi;
    }

    const PluginFunction kFn{"deg", 1, &deg};
}

CALC_EXPORT const PluginFunction* CalcGetPluginFunction() {
    return &kFn;
}