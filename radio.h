#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

//  uncomment exactly one mode below (do same in .c file):
#define NODE_TX  // compile as transmitter
// #define NODE_RX  // compile as receiver


//  Initialize the RFM69 radio module.
// This must be called once at startup. In RX mode, this also
//  switches the radio into receive mode immediately.

void radio_init(void);

// Debug: read register and print on LCD
void radio_debug_print_register(uint8_t addr);



#ifdef NODE_TX

// Transmit a null-terminated string over the radio.
// Blocks until transmission completes. The message length is taken
// from strlen(msg) and does not include the terminating '\0'.

void radio_send(const char *msg);
#else

//  Polls for incoming packets and displays them on the LCD.
// If a packet has been received, reads up to 31 bytes into a local buffer,
// NUL-terminates it, and writes it to the LCD. Automatically re-arms
//  the receiver for the next packet.

void radio_rx_poll(void);
#endif

void dio0_scope_pulse_loop(uint8_t ms_delay);

#endif // RADIO_H

