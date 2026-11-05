#include "../src/ao.h"
#include "../src/curve.h"
#include <cmath>
#include <iostream>
int main() {
    int failures = 0;
    auto check = [&](bool ok) { if (!ok) ++failures; };
    auto near = [](double a, double b) { return std::abs(a - b) < 1e-10; };
    Curve curve;
    check(AO(curve) == 0.0);
    curve.x.fill(1.5);
    check(near(AO(curve), 0));
    // x(y)=2y: upper area=0.75, lower area=0.25, hence AO=0.5.
    for (int i = 0; i <= Curve::kLast; ++i)
        curve.x[static_cast<std::size_t>(i)] = 2.0 * (1.0 - double(i) / Curve::kLast);
    check(near(AO(curve), 0.5));
    for (auto& x : curve.x) x = 2.0 - x;
    check(near(AO(curve), -0.5));
    curve.x.fill(0.0);
    curve.x.front() = 1.0;
    check(near(AO(curve), 1.0));
    curve.x.fill(0.0);
    curve.x.back() = 1.0;
    check(near(AO(curve), -1.0));
    check(near(GraphXFromFraction(0.5), 1));
    check(near(GraphXToFraction(1), 0.5));
    check(near(GraphXFromFraction(-1), 0) && near(GraphXFromFraction(2), 2));
    std::cout << failures << " failure(s)\n";
    return failures ? 1 : 0;
}
