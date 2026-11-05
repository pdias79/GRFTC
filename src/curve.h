#pragma once
#include <algorithm>
#include <array>
#include <cstddef>

// Normalized x values, ordered from the top (y=1) to the bottom (y=0).
// Fixed storage keeps drawing independent of window size and avoids allocations.
struct Curve {
    static constexpr int kLast = 1024;
    std::array<double, kLast + 1> x{};

    void DrawSegment(int first, double firstX, int last, double lastX) {
        first = std::clamp(first, 0, kLast);
        last = std::clamp(last, 0, kLast);
        firstX = std::clamp(firstX, 0.0, 1.0);
        lastX = std::clamp(lastX, 0.0, 1.0);
        if (first == last) {
            x[static_cast<std::size_t>(last)] = lastX;
            return;
        }
        const int begin = std::min(first, last);
        const int end = std::max(first, last);
        for (int i = begin; i <= end; ++i) {
            const double t = static_cast<double>(i - first) / (last - first);
            x[static_cast<std::size_t>(i)] = firstX + t * (lastX - firstX);
        }
    }
};
