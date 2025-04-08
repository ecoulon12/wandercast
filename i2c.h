#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// Initialize I2C with bit rate divider
void i2c_init(uint8_t bdiv);

// Perform I2C write/read transaction
uint8_t i2c_io(uint8_t device_addr,
               uint8_t *wp, uint16_t wn,
               uint8_t *rp, uint16_t rn);

#endif
