#include "../src/numeric.h"
#include <iostream>
#include "../src/RFTC.h"
int main() {
    int failures = 0;
    auto check = [&](bool success) { if (!success) ++failures; };
    double value = 0;
    check(ParseNumber(L"  -12,5  ", value) && value == -12.5);
    check(ParseNumber(L"1.25e2", value) && value == 125);
    for (const auto* invalid : {L"", L" ", L"-", L"1,2.3", L"12abc", L"nan", L"inf", L"1e9999"})
        check(!ParseNumber(invalid, value));
    char name[] = "CONFIG1";
    check(RFTC(name, 100, 290, 155, 1.4, 100, 1.0) == -190);
    check(RFTC(name, -10, 2.5, 155, 1.4, 100, 1.0) == -12.5);
    std::cout << failures << " failure(s)\n";
    return failures ? 1 : 0;
}
