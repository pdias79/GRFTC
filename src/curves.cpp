#include "curves.h"
#include "curve.h"
#include <cmath>

namespace {
constexpr double pi = 3.14159265358979323846;

// C++17 std::sin is not constexpr. Reflection reduces the Taylor argument
// to [-pi/2, pi/2]; ten terms are accurate to double precision in this range.
constexpr double SinConstexpr(double angle) {
    while (angle > pi) angle -= 2.0 * pi;
    while (angle < -pi) angle += 2.0 * pi;
    if (angle > pi / 2.0) angle = pi - angle;
    if (angle < -pi / 2.0) angle = -pi - angle;
    double term = angle;
    double result = term;
    for (int i = 1; i < 10; ++i) {
        term *= -angle * angle / ((2.0 * i) * (2.0 * i + 1.0));
        result += term;
    }
    return result;
}
constexpr double CurveConstexpr(double y, double alpha) {
    const double x = 0.2 + 1.5 * SinConstexpr(pi * (y + alpha * y * (1.0 - y)));
    return std::clamp(x, kGraphXMin, kGraphXMax);
}

// Same samples, clipping and trapezoid integration as the displayed curve/AO.
constexpr double AOForAlpha(double alpha) {
    double upper = 0.0;
    double lower = 0.0;
    double previous = CurveConstexpr(kGraphYMax, alpha);
    for (int row = 1; row <= Curve::kLast; ++row) {
        const double y = kGraphYMax - static_cast<double>(row)
            * (kGraphYMax - kGraphYMin) / Curve::kLast;
        const double current = CurveConstexpr(y, alpha);
        // The common factor dy/2 cancels in the AO ratio.
        if (row <= Curve::kLast / 2) upper += previous + current;
        else lower += previous + current;
        previous = current;
    }
    return upper + lower == 0.0 ? 0.0 : (upper - lower) / (upper + lower);
}
constexpr double FindAlpha(double targetAO) {
    // On [-1,0], AO decreases towards zero as alpha increases.
    double left = -1.0;
    double right = 0.0;
    for (int iteration = 0; iteration < 36; ++iteration) {
        const double middle = (left + right) / 2.0;
        if (AOForAlpha(middle) > targetAO) left = middle;
        else right = middle;
    }
    return (left + right) / 2.0;
}
constexpr double Abs(double x) { return x < 0.0 ? -x : x; }
static_assert(Curve::kLast % 2 == 0, "AO needs a sample at mid-height");
static_assert(AOForAlpha(-1.0) > 0.18 && Abs(AOForAlpha(0.0)) < 1e-10,
    "The graph limits must allow the requested AO targets in alpha [-1,0]");
constexpr double kAlphaP9 = FindAlpha(0.09);
constexpr double kAlphaP18 = FindAlpha(0.18);
static_assert(Abs(AOForAlpha(kAlphaP9) - 0.09) < 1e-10, "P9 calibration failed");
static_assert(Abs(AOForAlpha(kAlphaP18) - 0.18) < 1e-10, "P18 calibration failed");
}

// y=0 is the bottom of the graph, y=1 is the top.
double CurveWithAlpha(double y, double alpha) {
    return 0.2 + 1.5 * std::sin(pi * (y + alpha * y * (1.0 - y)));
}
double AlphaP9() { return kAlphaP9; }
double AlphaP18() { return kAlphaP18; }
double COS(double y) { return CurveWithAlpha(y, 0.0); }
double P9(double y) { return CurveWithAlpha(y, kAlphaP9); }
double P18(double y) { return CurveWithAlpha(y, kAlphaP18); }

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
