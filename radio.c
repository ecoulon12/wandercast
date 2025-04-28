#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
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
#define REG_OCP             0x13  // Over-current protection
#define REG_SYNC_CONFIG     0x2E  // Sync word and CRC settings
#define REG_SYNC_VALUE1     0x2F  // Sync word byte 1
#define REG_SYNC_VALUE2     0x30  // Sync word byte 2
#define REG_PACKETCONFIG1   0x37  // Packet engine settings
#define REG_PAYLOAD_LENGTH  0x38  // Payload length (fixed-length mode)
#define REG_IRQFLAGS1       0x27  // IRQ flags 1 (ModeReady, TxReady)
#define REG_IRQFLAGS2       0x28  // IRQ flags 2 (PacketSent, PayloadReady)
#define REG_FIFOTHRESH      0x35  // FIFO threshold & Tx start
#define REG_PARAMP          0x56  // PA ramp-up
#define REG_DIO_MAPPING1    0x25  // DIO mapping

#define TXSTARTCOND_FIRSTBYTE  (1<<6)
#define REG_IRQFLAGS1   0x27
#define IRQFLAGS1_TXREADY  (1<<5)
#define IRQFLAGS1_PLLLOCK  (1<<4)

// OPMODE values to put the radio into Standby, TX or RX
#define MODE_STANDBY   0x04  // Standby mode (ready to change between RX/TX)
#define MODE_TRANSMIT  0x0C  // Transmit mode
#define MODE_RECEIVE   0x10  // Receive mode


// SPI / CS pin definitions for AVR ATmega328P
#define RFM69_CS       PB2   // Chip Select pin number
#define RFM69_CS_PORT  PORTB // Port for CS
#define RFM69_CS_DDR   DDRB  // Data Direction Register for CS

// Forward declarations
static void spi_init(void);
static uint8_t spi_transfer(uint8_t out);
static void select_chip(void);
static void deselect_chip(void);
static uint8_t read_reg(uint8_t addr);
static void write_reg(uint8_t addr, uint8_t val);
static bool rfm69_init(void);
static void rfm69_start_receive(void);
static bool rfm69_available(void);
static uint8_t rfm69_receive(uint8_t *buf, uint8_t maxlen);
static void rfm69_send(const uint8_t *buf, uint8_t len);
static void verify_spi_config(void);
static void verify_radio_registers(void);

volatile bool packet_done = false;


//  Configure AVR SPI interface in master mode and set CS high.
//  MOSI = PB3, SCK = PB5, CS = PB2 are set as outputs.
//  SPI mode 0 (CPOL=0, CPHA=0), clock divider = F_CPU/16.
static void spi_init(void) {
    // Set MOSI (PB3), SCK (PB5), and CS (PB2) as outputs
    DDRB |= (1<<PB3) | (1<<PB5) | (1<<PB2);
    // Enable SPI, Master mode, clock divider = 16, mode 0
    // SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
    SPSR &= ~(1<<SPI2X);

    // Set MISO (PB4) as input
    DDRB &= ~(1<<PB4);
    // Deselect RFM69 by setting CS high
    PORTB |= (1<<PB2);

    // Configure PD2/INT0 as input
    DDRD &= ~(1<<PD2);
    // Enable pull-up if you like
    PORTD |=  (1<<PD2);

    // Trigger INT0 on rising edge
    EICRA  |= (1<<ISC01)|(1<<ISC00);
    // Enable INT0
    EIMSK  |= (1<<INT0);

    // Verify SPI configured correctly
    verify_spi_config();
}

ISR(INT0_vect) {
    // PacketSent just went high on DIO0
    // ... handle completion here ...
    // e.g. put radio back to standby:
    packet_done = true;
    // write_reg(REG_OPMODE, MODE_STANDBY);
}

// Check SPI mode bits and display on LCD
static void verify_spi_config(void) {
    uint8_t spcr = SPCR;
    lcd_clear_screen();
    // Check MSTR bit
    if (spcr & (1<<MSTR)) lcd_write_string("SPI: Master\n");
    else lcd_write_string("SPI: Slave\n");
    // Check CPOL & CPHA
    lcd_write_string((spcr & (1<<CPOL)) ? "CPOL=1 " : "CPOL=0 ");
    lcd_write_string((spcr & (1<<CPHA)) ? "CPHA=1" : "CPHA=0");
    _delay_ms(1000);
    lcd_clear_screen();
}

// static void diO0_int_check(void)
// {

// }

// Wrap SPI transfer with interrupts off to ensure atomicity
static uint8_t spi_transfer(uint8_t out) {
    cli();
    SPDR = out;
    while (!(SPSR & (1<<SPIF)));
    uint8_t in = SPDR;
    sei();
    return in;
}

static void select_chip(void) {
    PORTB &= ~(1<<PB2);
}
static void deselect_chip(void) {
    PORTB |= (1<<PB2);
}

static uint8_t read_reg(uint8_t addr) {
    select_chip();
    spi_transfer(addr & 0x7F);
    uint8_t val = spi_transfer(0x00);
    deselect_chip();
    return val;
}

static void write_reg(uint8_t addr, uint8_t val) {
    select_chip();
    spi_transfer(addr | 0x80);
    spi_transfer(val);
    deselect_chip();
}

// After initialization, read back key registers and display
static void verify_radio_registers(void) {
    const uint8_t regs[] = { REG_OPMODE, REG_FIFOTHRESH, REG_PA_LEVEL, 0x5A, 0x5C, REG_OCP };
    char buf[20];
    uint8_t i;
    for (i = 0; i < sizeof(regs); ++i) {
        uint8_t r = regs[i];
        uint8_t v = read_reg(r);
        lcd_clear_screen();
        snprintf(buf, sizeof(buf), "R%02X: %02X", r, v);
        lcd_write_string(buf);
        _delay_ms(2000);
    }
    lcd_clear_screen();
}

static bool rfm69_init(void) {
    spi_init();
    _delay_ms(10);

    write_reg(REG_OPMODE,    MODE_STANDBY);
    write_reg(REG_DATAMODUL,  0x00);
    write_reg(REG_FRF_MSB,    0xE4);
    write_reg(REG_FRF_MID,    0xC0);
    write_reg(REG_FRF_LSB,    0x00);
    write_reg(REG_BITRATE_MSB,0x1A);
    write_reg(REG_BITRATE_LSB,0x0B);
    write_reg(REG_FDEV_MSB,   0x03);
    write_reg(REG_FDEV_LSB,   0x33);
    write_reg(0x19,           0x4A);
    write_reg(0x2C,           0x00);
    write_reg(0x2D,           0x04);
    write_reg(REG_SYNC_CONFIG,  0x90);
    write_reg(REG_SYNC_VALUE1,  0x2D);
    write_reg(REG_SYNC_VALUE2,  0xD4);
    // write_reg(REG_PACKETCONFIG1, 0x30);
    // write_reg(REG_PACKETCONFIG1, (1<<7)|(1<<4));  // 0x90
    // // write_reg(REG_PAYLOAD_LENGTH, 0x40);  // max length, ignored in var-len
    // write_reg(REG_PAYLOAD_LENGTH, 0x40);
    
    // Fixed-length packets, CRC off
    write_reg(REG_PACKETCONFIG1, 0x00);
    // Tell the radio we’ll send exactly N bytes per packet
    // write_reg(REG_PAYLOAD_LENGTH, N);

    // write_reg(REG_FIFOTHRESH, TXSTARTCOND_FIRSTBYTE | 0x01);
    write_reg(REG_FIFOTHRESH, 0x0F);   // bit7 = 0  → start TX as soon as FIFO is not empty

    write_reg(REG_PA_LEVEL, 0x7F);
    write_reg(0x5A,         0x5D);
    write_reg(0x5C,         0x7C);
    write_reg(REG_OCP,      0x0F);
    write_reg(REG_PARAMP,   0x08);

    // uint8_t m = read_reg(0x25);
    // m = (m & 0x3F) | (2 << 6);
    // write_reg(0x25, m);
    write_reg(REG_DIO_MAPPING1, (read_reg(REG_DIO_MAPPING1) & 0x3F) | (2 << 6));

    uint8_t pc2 = read_reg(0x3D);
    write_reg(0x3D, pc2 | (1<<2));  // enable "start on first byte" there


    // Display register values for verification
    verify_radio_registers();

    return true;
}

static void rfm69_start_receive(void) {
    write_reg(REG_OPMODE, MODE_RECEIVE);
}

static bool rfm69_available(void) {
    return (read_reg(REG_IRQFLAGS2) & (1<<2)) != 0;
}

static uint8_t rfm69_receive(uint8_t *buf, uint8_t maxlen) {
    // ... unchanged ...
    return 0; // stub
}

// static void rfm69_send(const uint8_t *buf, uint8_t len) {
//     // write_reg(REG_DIO_MAPPING1, (read_reg(REG_DIO_MAPPING1) & 0x3F) | (2<<6));
//     // Preload FIFO
//     write_reg(REG_OPMODE, MODE_STANDBY);
//     // dynamically set the fixed‐length register to match len
//     write_reg(REG_PAYLOAD_LENGTH, len);
//     select_chip(); 
//     spi_transfer(0x80); 
//     spi_transfer(len);
//     uint8_t i;
//     for (i = 0; i < len; i++) spi_transfer(buf[i]);
//     deselect_chip();
//     // Start TX
//     write_reg(REG_OPMODE, MODE_TRANSMIT);
//     // while ((read_reg(REG_IRQFLAGS1) & (1<<7)) == 0);
//     // while ((read_reg(REG_IRQFLAGS2) & (1<<3)) == 0){ 
//     //     lcd_clear_screen();
//     //     lcd_write_string("In IRQFLAGS2");
//     //     _delay_ms(1000);
//     //     uint8_t val = read_reg(REG_IRQFLAGS2);
//     //     lcd_clear_screen();
//     //     snprintf(buf, 20, "Reg 0x%02X: 0x%02X", REG_IRQFLAGS2, val);
//     //     lcd_write_string(buf);
//     //     _delay_ms(2000); 
//     // };
//     while (!packet_done) {
//         // optionally sleep, or wdt_reset, or low-power here
//      }
 
//     write_reg(REG_OPMODE, MODE_STANDBY);
// }

static void rfm69_send(const uint8_t *buf, uint8_t len) {
    packet_done = false;                 // ① clear flag

    write_reg(REG_OPMODE, MODE_STANDBY);
    write_reg(REG_PAYLOAD_LENGTH, len);

    select_chip();
      spi_transfer(0x80);
      uint8_t i;
      for (i = 0; i < len; i++)
        spi_transfer(buf[i]);
    deselect_chip();

    write_reg(REG_OPMODE, MODE_TRANSMIT);  //  kick off TX

    radio_debug_print_register(REG_OPMODE); // should be 0x0C

    // wait for the PA/transmitter to be ready
    while ((read_reg(REG_IRQFLAGS1) & IRQFLAGS1_TXREADY) == 0) { 
        lcd_clear_screen();
        lcd_write_string("is TX ready");
        _delay_ms(2000); 
        lcd_clear_screen();
    }

    while ((read_reg(REG_IRQFLAGS1) & IRQFLAGS1_PLLLOCK) == 0) { 
        lcd_clear_screen();
        lcd_write_string("is PLLlock ready");
        _delay_ms(2000); 
        lcd_clear_screen();
    }

    lcd_clear_screen();
    lcd_write_string("was PLLlock");
    _delay_ms(2000); 
    // lcd_clear_screen();
    // ③ wait here for the ISR to fire
    while (!packet_done) {
      // optionally sleep here to save power
    }

    // ④ now that packet_done is true, switch back to standby
    write_reg(REG_OPMODE, MODE_STANDBY);
}


void radio_debug_print_register(uint8_t addr) {
    char buf[20];
    uint8_t val = read_reg(addr);
    lcd_clear_screen();
    snprintf(buf, 20, "Reg 0x%02X: 0x%02X", addr, val);
    lcd_write_string(buf);
    _delay_ms(2000); 
    lcd_clear_screen();
}


void radio_init(void) {
    rfm69_init();
#ifdef NODE_RX
    rfm69_start_receive();
#endif
}

#ifdef NODE_TX
void radio_send(const char *msg) {
    rfm69_send((const uint8_t*)msg, strlen(msg));
}
#else
void radio_rx_poll(void) {
    // ... unchanged ...
}
#endif


void dio0_scope_pulse_loop(uint8_t ms_delay)
{
    /* Make sure DIO0 is mapped to ModeReady (00 on bits 7-6).               *
     * After reset that’s already true, but this guarantees it.             */
    write_reg(REG_DIO_MAPPING1,
              (read_reg(REG_DIO_MAPPING1) & 0x3F) | (0 << 6));

              radio_debug_print_register(REG_DIO_MAPPING1);

    while (1)
    {
        /* ----- force SLEEP (clears ModeReady flag) ----- */
        write_reg(REG_OPMODE, 0x00);
        _delay_ms(ms_delay);          // plenty of settle time

        /* ----- jump to STANDBY → ModeReady LOW pulse ---- */
        write_reg(REG_OPMODE, MODE_STANDBY);
        _delay_ms(ms_delay);          // stay here so you can see gap
    }
}
