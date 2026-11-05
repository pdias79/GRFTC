#pragma once
#include <cerrno>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <string>

// Independent of the Windows UI so the future calculation can be tested separately.
inline bool ParseNumber(std::wstring text, double& value) {
    for (auto& c : text) if (c == L',') c = L'.';
    const wchar_t* start = text.c_str();
    while (std::iswspace(*start)) ++start;
    if (!*start) return false;
    errno = 0;
    wchar_t* end = nullptr;
    const double parsed = std::wcstod(start, &end);
    if (start == end || errno == ERANGE || !std::isfinite(parsed)) return false;
    while (std::iswspace(*end)) ++end;
    if (*end) return false;
    value = parsed;
    return true;
}
inline double EvaluateRftc(double power, double temperature) {
    return power - temperature;
}
