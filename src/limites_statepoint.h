#pragma once

struct LimitesDouble {
    double min;
    double max;
};

// Inclusive bounds. Change these values here and rebuild the application.
inline constexpr LimitesDouble kLimitesPuissance{10.0, 200.0};
inline constexpr LimitesDouble kLimitesTemperature{250.0, 350.0};
inline constexpr LimitesDouble kLimitesPression{130.0, 170.0};
inline constexpr LimitesDouble kLimitesFDH{1.0, 5.0};
inline constexpr LimitesDouble kLimitesFDHE1{1.00, 1.10};
// No range was specified for the relative flow: any finite double is accepted.
