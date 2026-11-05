#include "ao.h"
#include "curve.h"

// Integrate the piecewise-linear curve used by the renderer, by trapezoids.
double AO(const Curve& curve) {
    static_assert(Curve::kLast % 2 == 0, "A sample must lie on the mid-height");
    const double step = (kGraphYMax - kGraphYMin) / Curve::kLast;
    double upper = 0.0;
    double lower = 0.0;
    for (int i = 0; i < Curve::kLast; ++i) {
        const double area = 0.5 * (ClampGraphX(curve.x[static_cast<std::size_t>(i)])
            + ClampGraphX(curve.x[static_cast<std::size_t>(i) + 1])) * step;
        if (i < Curve::kLast / 2) upper += area;
        else lower += area;
    }
    const double total = upper + lower;
    return total == 0.0 ? 0.0 : (upper - lower) / total;
}
