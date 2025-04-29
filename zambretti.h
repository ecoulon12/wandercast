// zambretti.h

// #include <cstdint>
#include <stdio.h>
#ifndef ZAMBRETTI_H
#define ZAMBRETTI_H

typedef enum {
    PRESSURE_RISING,
    PRESSURE_STEADY,
    PRESSURE_FALLING
} PressureTrend;

// Predicts the weather based on pressure (in hPa) and trend
// Returns a string describing the forecast
// const char* zambretti_forecast(float pressure_hPa, PressureTrend trend);
PressureTrend determine_pressure_trend(const int pressure_log[], uint8_t pressure_index);

#endif // ZAMBRETTI_H
