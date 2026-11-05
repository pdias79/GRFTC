#include "../src/curve.h"
#include <cmath>
#include <iostream>
int main() {
    int failures = 0;
    auto check = [&](bool ok) { if (!ok) ++failures; };
    Curve c;
    for (double x : c.x) check(x == 0);
    c.DrawSegment(100, 0.2, 200, 0.8);
    check(std::abs(c.x[150] - 0.5) < 1e-12);
    check(c.x[99] == 0 && c.x[201] == 0);
    c.DrawSegment(200, 0.8, 100, 0.4); // reverse stroke replaces old values
    check(std::abs(c.x[150] - 0.6) < 1e-12);
    c.DrawSegment(150, 0.6, 150, 0.9); // horizontal movement: one x per y
    check(c.x[150] == 0.9);
    c.DrawSegment(-100, -1, Curve::kLast + 100, 3);
    check(c.x.front() == 0 && c.x.back() == 2);
    for (double x : c.x) check(x >= 0 && x <= 2);
    std::cout << failures << " failure(s)\n";
    return failures ? 1 : 0;
}
