#ifndef TINY_BME280_H
#define TINY_BME280_H

#include <stdint.h>

// Public functions
void bme280_init_sensor(void);
void bme280_trigger_measurement(void);
uint8_t bme280_is_measuring(void);

// Read compensated raw values (no floats!)
void bme280_read_environment(int32_t *temperature_x100, uint32_t *pressure_pa, uint32_t *humidity_x1024);

#endif

