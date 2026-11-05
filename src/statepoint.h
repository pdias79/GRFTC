#pragma once
#include <array>
#include "limites_statepoint.h"
#include "numeric.h"

struct ChampStatepoint {
    const wchar_t* nom;
    const wchar_t* initial;
    const LimitesDouble* limites;
};
inline constexpr std::array<ChampStatepoint, 6> kChamps{{
    {L"Puissance", L"100", &kLimitesPuissance},
    {L"Temp\u00e9rature", L"290", &kLimitesTemperature},
    {L"Pression", L"155", &kLimitesPression},
    {L"FDH", L"1.4", &kLimitesFDH},
    {L"D\u00e9bit relatif", L"100", nullptr},
    {L"FDHE1", L"1.00", &kLimitesFDHE1}
}};
inline bool ValidateField(std::size_t index, const std::wstring& text, double& value) {
    if (index >= kChamps.size() || !ParseNumber(text, value)) return false;
    const auto* range = kChamps[index].limites;
    return !range || (value >= range->min && value <= range->max);
}
