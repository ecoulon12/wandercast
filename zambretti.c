// zambretti.c

#include "zambretti.h"

const char* zambretti_forecast(float pressure_hPa, PressureTrend trend) {
    if (trend == PRESSURE_RISING) {
        if (pressure_hPa >= 1020.0f) {
            return "Fine Weather";
        } else if (pressure_hPa >= 1005.0f) {
            return "Fair Weather";
        } else {
            return "Unsettled, Improving";
        }
    } else if (trend == PRESSURE_STEADY) {
        if (pressure_hPa >= 1015.0f) {
            return "Fair Weather";
        } else {
            return "Unsettled";
        }
    } else if (trend == PRESSURE_FALLING) {
        if (pressure_hPa >= 1010.0f) {
            return "Fair, then Showers";
        } else if (pressure_hPa >= 995.0f) {
            return "Showers";
        } else {
            return "Rain";
        }
    } else {
        return "Unknown Trend";
    }
}
