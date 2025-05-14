
#include <util/delay.h>
#include <stdio.h>
#include "zambretti.h"
#define MAX_MEASUREMENTS 6

PressureTrend determine_pressure_trend(const int pressure_log[], uint8_t pressure_index) {
    pressure_index = 0;   
    int current = pressure_log[(pressure_index - 1 + MAX_MEASUREMENTS) % MAX_MEASUREMENTS];
    int previous = pressure_log[(pressure_index - 2 + MAX_MEASUREMENTS) % MAX_MEASUREMENTS];

    int32_t delta = current - previous;

    if (delta > 20) { // If pressure rose more than ~2 hPa
        return PRESSURE_RISING;
    } else if (delta < -20) { // If pressure dropped more than ~2 hPa
        return PRESSURE_FALLING;
    } else {
        return PRESSURE_STEADY;
    }
}


const char* zambretti_forecast(float pressure_hPa, PressureTrend trend) {
    int easterly = (wind_dir_deg >= 90 && wind_dir_deg <= 150);

    if (trend == PRESSURE_RISING) {
        if (pressure_hPa >= 1020) {
            return "Fine Weather";
        } else if (pressure_hPa >= 1005) {
            return "Fair Weather";
        } else {
            return easterly ? "Unsettled" : "Unsettled, Improving";
        }
    } else if (trend == PRESSURE_STEADY) {
        if (pressure_hPa >= 1015) {
            return "Fair Weather";
        } else {
            return "Unsettled";
        }
    } else if (trend == PRESSURE_FALLING) {
        if (pressure_hPa >= 1010) {
            return easterly ? "Showers" : "Fair, then Showers";
        } else if (pressure_hPa >= 995) {
            return "Showers";
        } else {
            return "Rain";
        }
    } else {
        return "Unknown Trend";
    }
}
