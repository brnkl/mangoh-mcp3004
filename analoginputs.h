#ifndef ANALOGINPUTS_H
#define ANALOGINPUTS_H

#include "legato.h"

#define N_CHANNELS 4

le_result_t analog_readVoltage(int chan, double* voltage);
double analog_avgReadings(double* readings, int nReadings);
LE_SHARED le_result_t analog_takeReadings(double* val, int chan);
LE_SHARED le_result_t analog_readInputs(double* aIn, int nAIn);
void analog_setup();

#endif  // ANALOGINPUTS_H
