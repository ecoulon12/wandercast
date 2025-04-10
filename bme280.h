#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include <avr/io.h>

// Define the I2C address for your LCD (8-bit format)
#define BME280_I2C_ADDR (0x77 << 1)
#define BME280_ID_REG_ADDR 0xD0
#define BME280_RESET_REG_ADDR 0xE0
#define BME280_CTRL_HUM_REG_ADDR 0xF2
#define BME280_STATUS_REG_ADDR 0xF3
#define BME280_CTRL_MEAS_REG_ADDR 0xF4
#define BME280_CONFIG_REG_ADDR 0xF5

//raw pressure measurement output data registers
#define BME280_PRES_MSB_REG_ADDR 0xF7
#define BME280_PRES_LSB_REG_ADDR 0xF8
#define BME280_PRES_XLSB_REG_ADDR 0xF9

// raw temperature measurement output data registers
#define BME280_TEMP_MSB_REG_ADDR 0xFA
#define BME280_TEMP_LSB_REG_ADDR 0xFB
#define BME280_TEMP_XLSB_REG_ADDR 0xFC

//  raw humidity measurement output data registers
#define BME280_HUM_MSB_REG_ADDR 0xFD
#define BME280_HUM_LSB_REG_ADDR 0xFE

// Masks to help do config stuff
#define BME280_OSRS_T_MASK  0xE0
#define BME280_OSRS_P_MASK  0x1C
#define BME280_MODE_MASK    0x03


// Public functions
void bme280_init(void);
void bme280_get_id();
uint8_t bme280_read_reg(uint8_t);
uint8_t bme280_get_status();
void bme280_print_reg(uint8_t);

#endif
