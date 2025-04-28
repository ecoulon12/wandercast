#include "tiny_bme280.h"
#include "i2c.h"
#include <util/delay.h>
#include <stdio.h>

#define BME280_ADDR (0x76 << 1)  // 8-bit I2C address

// BME280 register addresses
#define REG_ID         0xD0
#define REG_RESET      0xE0
#define REG_CTRL_HUM   0xF2
#define REG_STATUS     0xF3
#define REG_CTRL_MEAS  0xF4
#define REG_CONFIG     0xF5
#define REG_PRESS_MSB  0xF7

// Calibration data struct
static struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} calib_data;

static int32_t t_fine = 0;

// Utility functions
static uint8_t read_reg(uint8_t reg) {
    uint8_t val;
    i2c_io(BME280_ADDR, &reg, 1, &val, 1);
    return val;
}

static void read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_io(BME280_ADDR, &reg, 1, buf, len);
}

static void write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_io(BME280_ADDR, buf, 2, NULL, 0);
}

static uint16_t concat_u16(uint8_t msb, uint8_t lsb) {
    return ((uint16_t)msb << 8) | lsb;
}

static int16_t concat_s16(uint8_t msb, uint8_t lsb) {
    return (int16_t)(((uint16_t)msb << 8) | lsb);
}

// Init sensor
void bme280_init_sensor(void) {
    uint8_t buf[26];

    // Soft reset (optional)
    write_reg(REG_RESET, 0xB6);
    _delay_ms(5);

    // Read calibration data
    read_regs(0x88, buf, 26);
    calib_data.dig_T1 = concat_u16(buf[1], buf[0]);
    calib_data.dig_T2 = concat_s16(buf[3], buf[2]);
    calib_data.dig_T3 = concat_s16(buf[5], buf[4]);
    calib_data.dig_P1 = concat_u16(buf[7], buf[6]);
    calib_data.dig_P2 = concat_s16(buf[9], buf[8]);
    calib_data.dig_P3 = concat_s16(buf[11], buf[10]);
    calib_data.dig_P4 = concat_s16(buf[13], buf[12]);
    calib_data.dig_P5 = concat_s16(buf[15], buf[14]);
    calib_data.dig_P6 = concat_s16(buf[17], buf[16]);
    calib_data.dig_P7 = concat_s16(buf[19], buf[18]);
    calib_data.dig_P8 = concat_s16(buf[21], buf[20]);
    calib_data.dig_P9 = concat_s16(buf[23], buf[22]);
    calib_data.dig_H1 = buf[25];

    read_regs(0xE1, buf, 7);
    calib_data.dig_H2 = concat_s16(buf[1], buf[0]);
    calib_data.dig_H3 = buf[2];
    calib_data.dig_H4 = (int16_t)((buf[3] << 4) | (buf[4] & 0x0F));
    calib_data.dig_H5 = (int16_t)((buf[5] << 4) | (buf[4] >> 4));
    calib_data.dig_H6 = (int8_t)buf[6];

    // Setup oversampling
    write_reg(REG_CTRL_HUM, 0x01); // Humidity oversampling x1
    write_reg(REG_CTRL_MEAS, 0x27); // Temp and Pressure oversampling x1, mode = normal
    write_reg(REG_CONFIG, 0x00); // IIR filter off, standby 0.5ms
}

// Trigger a forced measurement
void bme280_trigger_measurement(void) {
    write_reg(REG_CTRL_MEAS, 0x25); // Forced mode
}

// Check if measuring
uint8_t bme280_is_measuring(void) {
    uint8_t status = read_reg(REG_STATUS);
    return (status & 0x08); // measuring bit
}

// Read and compensate data
void bme280_read_environment(int32_t *temperature_x100, uint32_t *pressure_pa, uint32_t *humidity_x1024) {
    uint8_t buf[8];
    read_regs(REG_PRESS_MSB, buf, 8);

    uint32_t adc_P = (((uint32_t)buf[0]) << 12) | (((uint32_t)buf[1]) << 4) | (buf[2] >> 4);
    uint32_t adc_T = (((uint32_t)buf[3]) << 12) | (((uint32_t)buf[4]) << 4) | (buf[5] >> 4);
    uint32_t adc_H = (((uint32_t)buf[6]) << 8) | buf[7];

    // Temperature compensation (integer) 
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * ((int32_t)calib_data.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) *
                    ((int32_t)calib_data.dig_T3)) >> 14;
    t_fine = var1 + var2;
    *temperature_x100 = (t_fine * 5 + 64) >> 7;  // °C × 100

    // Pressure compensation
    int64_t var1p, var2p, p;
    var1p = ((int64_t)t_fine) - 128000;
    var2p = var1p * var1 * (int64_t)calib_data.dig_P6;
    var2p = var2p + ((var1p * (int64_t)calib_data.dig_P5) << 17);
    var2p = var2p + (((int64_t)calib_data.dig_P4) << 35);
    var1p = ((var1p * var1p * (int64_t)calib_data.dig_P3) >> 8) +
        ((var1p * (int64_t)calib_data.dig_P2) << 12);
    var1p = (((((int64_t)1) << 47) + var1p)) * ((int64_t)calib_data.dig_P1) >> 33;

    if (var1p == 0) {
        *pressure_pa = 0;
    } else {
        p = 1048576 - adc_P;
        p = (((p << 31) - var2) * 3125) / var1;
        var1p = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
        var2p = (((int64_t)calib_data.dig_P8) * p) >> 19;
        p = ((p + var1p + var2p) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
        *pressure_pa = (uint32_t)(p >> 8);
    }

    // --- Humidity compensation (integer) ---
    int32_t v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib_data.dig_H4) << 20) -
                   (((int32_t)calib_data.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                 (((((((v_x1_u32r * ((int32_t)calib_data.dig_H6)) >> 10) *
                      (((v_x1_u32r * ((int32_t)calib_data.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                   ((int32_t)calib_data.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calib_data.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    *humidity_x1024 = (uint32_t)(v_x1_u32r >> 12); // %RH × 1024
}



