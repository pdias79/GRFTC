#include "../src/curves.h"
#include "../src/curve.h"
#include "../src/ao.h"
#include <cmath>
#include <iostream>
int main() {
    int failures = 0;
    auto check = [&](bool ok) { if (!ok) ++failures; };
    auto near = [](double a, double b) { return std::abs(a - b) < 1e-10; };
    check(near(COS(0), 0.2) && near(COS(0.5), 1.7) && near(COS(1), 0.2));
    check(near(P9(0), 0.2) && near(P9(0.5), 2.2) && near(P9(1), 1.2));
    check(near(P18(0), 0.2) && near(P18(0.5), 2.7) && near(P18(1), 2.2));
    Curve curve;
    check(ApplyCurve(curve, CurveShape::Cos));
    check(near(curve.x.front(), 0.2) && near(curve.x.back(), 0.2));
    check(near(curve.x[Curve::kLast / 2], 1.7));
    check(near(AO(curve), 0));
    auto saved = curve.x;
    check(!ApplyCurve(curve, CurveShape::Custom) && curve.x == saved);
    check(ApplyCurve(curve, CurveShape::P9));
    check(near(curve.x.front(), 1.2) && near(curve.x.back(), 0.2));
    check(near(curve.x[Curve::kLast / 2], kGraphXMax));
    check(AO(curve) > 0);
    check(ApplyCurve(curve, CurveShape::P18));
    check(near(curve.x.front(), kGraphXMax) && near(curve.x.back(), 0.2));
    check(AO(curve) > 0);
    for (const double x : curve.x) check(x >= kGraphXMin && x <= kGraphXMax);
    curve.DrawSegment(100, 0.8, 400, 1.0);
    saved = curve.x;
    check(!ApplyCurve(curve, CurveShape::Custom) && curve.x == saved);
    check(ApplyCurve(curve, CurveShape::Cos));
    check(curve.x != saved && near(AO(curve), 0));
    std::cout << failures << " failure(s)\n";
    return failures ? 1 : 0;
}
