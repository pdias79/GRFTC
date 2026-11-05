#include "RFTC.h"

double RFTC(char* nom_configuration, double puissance, double temperature,
            double pression, double fdh, double debit, double fhde1) {
    // Temporary formula. The remaining inputs are available for the next model.
    (void)nom_configuration;
    (void)pression;
    (void)fdh;
    (void)debit;
    (void)fhde1;
    return puissance - temperature;
}
