#include "RFTC.h"
#include <cstring>

double RFTC(char* nom_configuration, double puissance, double temperature,
            double pression, double fdh, double debit, double fhde1) {
    (void)debit;
    (void)fhde1;
    if (nom_configuration && std::strcmp(nom_configuration, "CONFIG1") == 0)
        return puissance - temperature;
    return puissance * fdh - temperature + pression;
}
