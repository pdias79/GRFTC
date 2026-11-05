#include "initial_curve.h"
#include "curve.h"
#include <cmath>

// Edit this function to choose the starting curve. y=0 is bottom, y=1 is top.
double InitialCurve(double y) {
    constexpr double pi = 3.14159265358979323846;
    return 0.2 + 1.5 * std::sin(2.0 * pi * y);
}

void InitializeCurve(Curve& curve) {
    for (int row = 0; row <= Curve::kLast; ++row) {
        const double y = kGraphYMax - static_cast<double>(row)
            * (kGraphYMax - kGraphYMin) / Curve::kLast;
        // Negative/out-of-scale values are clipped to the visible domain.
        curve.x[static_cast<std::size_t>(row)] = ClampGraphX(InitialCurve(y));
    }
}
