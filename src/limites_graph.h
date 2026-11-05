#pragma once
#include <algorithm>
#include <cmath>

inline constexpr double kGraphXMin = 0.0;
inline constexpr double kGraphXMax = 2.0;
inline constexpr double kGraphYMin = 0.0;
inline constexpr double kGraphYMax = 1.0;
static_assert(kGraphXMax > kGraphXMin && kGraphYMax > kGraphYMin,
              "Graph maximums must exceed minimums");
static_assert(kGraphXMin == 0.0, "The filled area starts at x=0");
inline double ClampGraphX(double x) {
    return std::isfinite(x) ? std::clamp(x, kGraphXMin, kGraphXMax) : kGraphXMin;
}
inline double GraphXFromFraction(double fraction) {
    return kGraphXMin + std::clamp(fraction, 0.0, 1.0) * (kGraphXMax - kGraphXMin);
}
inline double GraphXToFraction(double x) {
    return (ClampGraphX(x) - kGraphXMin) / (kGraphXMax - kGraphXMin);
}
