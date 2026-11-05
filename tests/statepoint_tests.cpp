#include "../src/statepoint.h"
#include "../src/configurations.h"
#include <iostream>
#include <sstream>
int main() {
    int failures = 0;
    auto check = [&](bool ok) { if (!ok) ++failures; };
    double value = 0;
    for (std::size_t i = 0; i < kChamps.size(); ++i) {
        check(ValidateField(i, kChamps[i].initial, value));
        for (const auto* bad : {L"", L"abc", L"nan", L"inf", L"1e9999"})
            check(!ValidateField(i, bad, value));
        if (const auto* limits = kChamps[i].limites) {
            check(ValidateField(i, std::to_wstring(limits->min), value));
            check(ValidateField(i, std::to_wstring(limits->max), value));
            check(!ValidateField(i, std::to_wstring(limits->min - 0.01), value));
            check(!ValidateField(i, std::to_wstring(limits->max + 0.01), value));
        }
    }
    check(ValidateField(3, L"1,4", value) && value == 1.4);
    check(ValidateField(5, L"1,10", value) && value == 1.1);
    check(!ValidateField(4, L"-1000000", value));
    check(ValidateField(4, L"30", value));
    check(ValidateField(4, L"110", value));
    check(!ValidateField(4, L"29.99", value));
    check(!ValidateField(4, L"110.01", value));
    check(!ValidateField(6, L"100", value));
    std::istringstream config("\xef\xbb\xbf# Configurations\r\n CONFIG1 \r\n\nCONFIG6\n  Other name  \r\n");
    const auto names = ReadConfigurations(config);
    check(names.size() == 3);
    if (names.size() == 3) {
        check(names[0] == "CONFIG1");
        check(names[1] == "CONFIG6");
        check(names[2] == "Other name");
    }
    std::istringstream empty("# comment\n\n \t\r\n");
    check(ReadConfigurations(empty).empty());
    std::cout << failures << " failure(s)\n";
    return failures ? 1 : 0;
}
