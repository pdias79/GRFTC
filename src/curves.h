#pragma once
#include <array>
struct Curve;

enum class CurveShape { Cos, P9, P18, Custom };
inline constexpr std::array<const wchar_t*, 4> kCurveNames{L"COS", L"P9", L"P18", L"custom"};

double COS(double y);
double P9(double y);
double P18(double y);
// Returns false for custom (no changes); presets replace the current curve.
bool ApplyCurve(Curve& curve, CurveShape shape);
