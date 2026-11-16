#include "PluginAPI.h"
#include <cmath>
#include <stdexcept>

namespace {
    double lnSafe(double x) {
        if (!std::isfinite(x)) throw std::runtime_error("ln: argument is not finite");
        if (x <= 0.0) throw std::runtime_error("ln: argument must be > 0");
        return std::log(x);
    }
    const PluginFunction kFn{"ln", 1, &lnSafe};
}

CALC_EXPORT const PluginFunction* CalcGetPluginFunction() {
    return &kFn;
}