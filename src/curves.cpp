#include "curves.h"
#include "curve.h"
#include <cmath>

namespace {
constexpr double pi = 3.14159265358979323846;
}

// y=0 is the bottom of the graph, y=1 is the top.
double COS(double y) { return 0.2 + 1.5 * std::sin(pi * y); }
double P9(double y) { return 0.2 + y + 1.5 * std::sin(pi * y); }
double P18(double y) { return 0.2 + 2.0 * y + 1.5 * std::sin(pi * y); }

bool ApplyCurve(Curve& curve, CurveShape shape) {
    double (*formula)(double) = nullptr;
    switch (shape) {
    case CurveShape::Cos: formula = COS; break;
    case CurveShape::P9: formula = P9; break;
    case CurveShape::P18: formula = P18; break;
    case CurveShape::Custom: return false;
    default: return false;
    }
    for (int row = 0; row <= Curve::kLast; ++row) {
        const double y = kGraphYMax - static_cast<double>(row)
            * (kGraphYMax - kGraphYMin) / Curve::kLast;
        curve.x[static_cast<std::size_t>(row)] = ClampGraphX(formula(y));
    }
    return true;
}
