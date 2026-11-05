#include "../src/numeric.h"
#include <iostream>
int main() {
    int failures = 0;
    auto check = [&](bool success) { if (!success) ++failures; };
    double value = 0;
    check(ParseNumber(L"  -12,5  ", value) && value == -12.5);
    check(ParseNumber(L"1.25e2", value) && value == 125);
    for (const auto* invalid : {L"", L" ", L"-", L"1,2.3", L"12abc", L"nan", L"inf", L"1e9999"})
        check(!ParseNumber(invalid, value));
    check(EvaluateRftc(100, 25) == 75);
    check(EvaluateRftc(-10, 2.5) == -12.5);
    std::cout << failures << " failure(s)\n";
    return failures ? 1 : 0;
}
