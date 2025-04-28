#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "lcd.h"
#include "radio.h"
#include <stdio.h>


//  uncomment exactly one mode below:
#define NODE_TX   // Compile as transmitter
// #define NODE_RX   // Compile as receiver


// RFM69 register addresses 
#define REG_OPMODE          0x01  // Operating mode and sequencing
#define REG_DATAMODUL       0x02  // Data mode (packet/continuous) and modulation
#define REG_BITRATE_MSB     0x03  // Bitrate configuration MSB
#define REG_BITRATE_LSB     0x04  // Bitrate configuration LSB
#define REG_FDEV_MSB        0x05  // Frequency deviation MSB
#define REG_FDEV_LSB        0x06  // Frequency deviation LSB
#define REG_FRF_MSB         0x07  // RF carrier frequency MSB
#define REG_FRF_MID         0x08  // RF carrier frequency MID
#define REG_FRF_LSB         0x09  // RF carrier frequency LSB
#define REG_PA_LEVEL        0x11  // Power amplifier configuration
#define REG_SYNC_CONFIG     0x2E  // Sync word and CRC settings
#define REG_SYNC_VALUE1     0x2F  // Sync word byte 1
#define REG_SYNC_VALUE2     0x30  // Sync word byte 2
#define REG_PACKETCONFIG1   0x37  // Packet engine settings
#define REG_PAYLOAD_LENGTH  0x38  // Payload length (fixed-length mode)
#define REG_IRQFLAGS2       0x28  // IRQ flags 2 (PacketSent, PayloadReady)

// OPMODE values to put the radio into Standby, TX or RX
#define MODE_STANDBY   0x04  // Standby mode (ready to change between RX/TX)
#define MODE_TRANSMIT  0x0C  // Transmit mode
#define MODE_RECEIVE   0x10  // Receive mode

// SPI / CS pin definitions for AVR ATmega328P
#define RFM69_CS       PB2   // Chip Select pin number
#define RFM69_CS_PORT  PORTB // Port for CS
#define RFM69_CS_DDR   DDRB  // Data Direction Register for CS


//  Configure AVR SPI interface in master mode and set CS high.
//  MOSI = PB3, SCK = PB5, CS = PB2 are set as outputs.
//  SPI clock is F_CPU/16.
static void spi_init(void) {
    // Set MOSI (PB3), SCK (PB5), and CS (PB2) as outputs
    DDRB |= (1<<PB3) | (1<<PB5) | (1<<PB2);
    // Enable SPI, Master mode, clock divider = 16
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);

    // Set MISO (PB4) as input
    DDRB &= ~(1<<PB4);
    
    // Deselect RFM69 by setting CS high
    PORTB |= (1<<PB2);
}


//  Transfer one byte over SPI and receive response.
// return Byte read from MISO
static uint8_t spi_transfer(uint8_t out) {
    SPDR = out;
    // Wait for transfer complete flag
    while (!(SPSR & (1<<SPIF)));
    return SPDR;
}


//  Assert the RFM69 chip select (active low).
static void select_chip(void) {
    PORTB &= ~(1<<PB2);
}


//  Release the RFM69 chip select.
static void deselect_chip(void) {
    PORTB |= (1<<PB2);
}


// Read a single register from the RFM69 via SPI.
//  return Value read from register
static uint8_t read_reg(uint8_t addr) {
    select_chip();
    spi_transfer(addr & 0x7F);  // MSB=0 for read
    uint8_t val = spi_transfer(0x00);
    deselect_chip();
    return val;
}


//  Write a value to a single RFM69 register via SPI.
static void write_reg(uint8_t addr, uint8_t val) {
    select_chip();
    spi_transfer(addr | 0x80);  // MSB=1 for write
    spi_transfer(val);
    deselect_chip();
}


//  Initialize the RFM69 registers for basic operation.
// Puts radio in standby
// Sets FSK packet mode, frequency, bitrate, deviation
//  Configures PA, sync word, CRC, variable packet length
static bool rfm69_init(void) {
    spi_init();
    _delay_ms(10);  // wait for POR and PLL

    // 1) Standby mode (sequencer ON)
    write_reg(REG_OPMODE, MODE_STANDBY);

    // 2) Packet mode, FSK, no shaping
    write_reg(REG_DATAMODUL, 0x00);

    // 3) Bitrate ≃55.5 kbps  (32 MHz / 0x1A0B)
    write_reg(REG_BITRATE_MSB, 0x1A);
    write_reg(REG_BITRATE_LSB, 0x0B);

    // 4) Fdev ≃50 kHz  (0x0333 → 819+51 = 870 × 61 Hz)
    write_reg(REG_FDEV_MSB,    0x03);
    write_reg(REG_FDEV_LSB,    0x33);

    // 5) Carrier freq = 915 MHz (0xE4C000)
    write_reg(REG_FRF_MSB,     0xE4);
    write_reg(REG_FRF_MID,     0xC0);
    write_reg(REG_FRF_LSB,     0x00);

    // 6) RX bandwidth ≃125 kHz
    write_reg(0x19 /*REG_RXBW*/, 0x4A);

    // 7) Preamble length = 4 bytes
    write_reg(0x2C /*REG_PREAMBLE_MSB*/, 0x00);
    write_reg(0x2D /*REG_PREAMBLE_LSB*/, 0x04);

    // 8) Sync = 2 bytes, 0x2D 0xD4, zero tolerance
    write_reg(REG_SYNC_CONFIG,   0x90);
    write_reg(REG_SYNC_VALUE1,   0x2D);
    write_reg(REG_SYNC_VALUE2,   0xD4);

    // 9) PacketConfig1 = var-len + CRC on (no Manchester)
    write_reg(REG_PACKETCONFIG1, 0xD0);
    //    max payload = 64
    write_reg(REG_PAYLOAD_LENGTH, 0x40);

    // 10) PA1+PA2 on, Power = 31 (high-power HCW)
    write_reg(REG_PA_LEVEL,      0x7F);
    //    Test registers to unlock +20 dBm mode
    write_reg(0x5A /*TestPa1*/,  0x5D);
    write_reg(0x5C /*TestPa2*/,  0x7C);

    return true;
}


// Switch the radio into continuous receive mode.
static void rfm69_start_receive(void) {
    write_reg(REG_OPMODE, MODE_RECEIVE);
}


// Returns true if a complete packet is waiting in the FIFO (PayloadReady = bit2)
static bool rfm69_available(void) {
    // IRQFLAGS2 bit2 = PayloadReady
    return (read_reg(REG_IRQFLAGS2) & (1<<2)) != 0;
}


// Read a variable‐length packet into buf, up to maxlen.
// Returns the actual payload length.
static uint8_t rfm69_receive(uint8_t *buf, uint8_t maxlen) {
    uint8_t len;

    // Read first byte from FIFO = packet length
    select_chip();
      spi_transfer(0x00);           // FIFO read
      len = spi_transfer(0x00);     // first byte = length
      if (len > maxlen) len = maxlen;
      for (uint8_t i = 0; i < len; i++) {
        buf[i] = spi_transfer(0x00);
      }
    deselect_chip();

    // Return to standby 
    write_reg(REG_OPMODE, MODE_STANDBY);
    return len;
}

// Send buf[0..len-1] as a variable‐length packet.
// We prefix the FIFO with `len`, then the data bytes themselves.
static void rfm69_send(const uint8_t *buf, uint8_t len) {
    // Go to Standby
    write_reg(REG_OPMODE, MODE_STANDBY);

    // Write (len + data) into the FIFO
    select_chip();
      spi_transfer(0x80);       // FIFO write
      spi_transfer(len);        // first byte = length
      for (uint8_t i = 0; i < len; i++) {
        spi_transfer(buf[i]);   // then the payload
      }
    deselect_chip();

    // Kick off TX
    write_reg(REG_OPMODE, MODE_TRANSMIT);
    // Wait for PacketSent (IRQFLAGS2 bit3)
    while ((read_reg(REG_IRQFLAGS2) & (1<<3)) == 0) { }
    // Back to standby
    write_reg(REG_OPMODE, MODE_STANDBY);
}

void radio_debug_print_register(uint8_t addr) {
    char buf[20];
    uint8_t val = read_reg(addr);

    lcd_clear_screen();
    snprintf(buf, sizeof(buf), "Reg 0x%02X: 0x%02X", addr, val);
    lcd_write_string(buf);
    _delay_ms(2000); 
    lcd_clear_screen();
}


// Initialize the radio driver and, in RX mode, start receiving.
void radio_init(void) {
    rfm69_init();
#ifdef NODE_RX
    rfm69_start_receive();
#endif
}

#ifdef NODE_TX


//  Transmit a null-terminated ASCII string.
void radio_send(const char *msg) {
    rfm69_send((const uint8_t*)msg, strlen(msg));
}

#else  // NODE_RX

// Poll for an incoming packet; if available, display it on the LCD.
// Checks payload-ready flag, reads up to 31 bytes, NUL-terminates,
// shows on LCD, then re-arms receiver.
void radio_rx_poll(void) {
    if (!rfm69_available()) return;

    char buf[32];
    uint8_t len = rfm69_receive((uint8_t*)buf, sizeof(buf)-1);
    buf[len] = '\0';

    lcd_clear_screen();
    lcd_write_string(buf);

    // Re-enter receive mode for the next packet
    rfm69_start_receive();
}

#endif  // NODE_RX
