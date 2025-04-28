#ifndef WEATHERSENSORS_H
#define WEATHERSENSORS_H

#include <stdint.h>
#include <avr/io.h>

//Public functions
void weatherSensors_init();
int windVane();
int windSpeed();
int rain_um();

#endif