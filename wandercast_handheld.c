/********************************************
*
*  Name: Erin Coulon, Vic Nunez, Drew Uramoto
*  Section: EE459L
*  Assignment: Wandercast
*
********************************************/

#define NULL 
#define FOSC 9830400 // Clock frequency = Oscillator freq .
#define FOSC1 7372800 
#define BDIV ( FOSC1 / 200000 - 16) / 2 + 1

#define F_CPU 7372800UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"
#include "bme280.h"
#include "weatherSensors.h"
#include "radio.h"     
#include <stddef.h>

// === New: Global variable to count pulses ===
volatile uint16_t pulse_count = 0;

// Function prototypes
void io_pin_init();
void lcd_init();
void external_interrupt_init();
void send_forced_signal();
void send_forced_signal();

const char* forecast_lookup[] = {
    "Unknown", // 0 pulses
    "Fine Weather", // 1 pulse
    "Fair Weather", // 2 pulses
    "Unsettled, Improving", // 3
    "Fair Weather (falling)", // 4
    "Unsettled (falling)", // 5
    "Fair, then Showers", // 6
    "Showers", // 7
    "Rain" // 8
};

// const char* get_forecast_from_pulse_count(uint16_t pulse_count) {
//     if (pulse_count >= 1 && pulse_count <= 8) {
//         return forecast_lookup[pulse_count];
//     } else {
//         return "Unknown Forecast";
//     }
// }


int main(void)
{
    TWSR = 0; // Set prescaler for 1
    TWBR = BDIV ; // Set bit rate register

    // Init hardware
    io_pin_init();
    external_interrupt_init();  // initialize interrupt to count 
    radio_init();
    lcd_init();

    // Debug prints for radio
    lcd_clear_screen();
    lcd_write_string("Past radio init!");
    _delay_ms(1000);
    radio_debug_print_register(0x2F); // Should print 0x2D
    radio_debug_print_register(0x30); // Should print 0xD4
    radio_debug_print_register(0x01); // Should print 0x04

    lcd_clear_screen();
    lcd_write_string("Start of the program! -- HANDHELD");
    _delay_ms(1000);
    lcd_clear_screen();

    bme280_init();
    weatherSensors_init();
    lcd_clear_screen();
    
    // Variables
    int windDir;
    int windSpd;
    char buffer[16]; // For displaying pulse count

    while(1) {

        // Force sampling if button pressed
        if (!(PIND & (1<<PD3))){
            lcd_clear_screen();
            lcd_move_cursor(0,0);
            send_forced_signal();
            // lcd_move_
            lcd_write_string("button press!!!");
        }else{
            lcd_clear_screen();
        }
    
        // // If we got a new pulse since last check, reset the timer
        // if (pulse_count != last_pulse_count) {
        //     no_pulse_timer_ms = 0;
        //     last_pulse_count = pulse_count;
        // } else {
        //     no_pulse_timer_ms += 500; // Every loop is about 500ms
        // }
    
        // // If no pulse has arrived for 2 seconds, forecast is ready
        // if (no_pulse_timer_ms >= 2000 && pulse_count > 0) {
        //     forecast_ready = 1;
        // }
    
        // // LCD display
        // lcd_clear_screen();
        // lcd_set_cursor(0, 0);
    
        // if (forecast_ready) {
        //     const char* forecast = get_forecast_from_pulse_count(pulse_count);
        //     lcd_write_string(forecast);
    
        //     // Reset for next message
        //     pulse_count = 0;
        //     last_pulse_count = 0;
        //     no_pulse_timer_ms = 0;
        //     forecast_ready = 0;
        // } else {
        //     char buffer[16];
        //     snprintf(buffer, sizeof(buffer), "Pulses: %u", pulse_count);
        //     lcd_write_string(buffer);
        // }
    
        // #ifdef RX_MODE
        //     radio_rx_poll();  // Poll and display
        // #endif
    
        _delay_ms(500);
    }
    
    
    return 0;   /* never reached */
}

// === IO Initialization ===
void io_pin_init() {
    // Set up pins
    PORTD &= ~(0xFF);
    DDRD = 0xEF;  // PD3 button input, PD4 pulse output, rest output
    DDRD &= ~(1<<PD3);
    PORTD |= (1 << PD3); // pull-up resistor for forced button

    DDRC = (1 << PC5 | 1 << PC4 | 1 << PC0); // 0011 0001 PC3,2,1 to input
    PORTC &= ~(0x3F);
    PORTB &= ~(0xBF);

    // Ensure PD2 (INT0/DIO0) remains input with pull-up
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    DDRC &= ~(1 << PC0);   // make EN a high-impedance input
    PORTC |= (1 << PC0);   // pull it up to VIN so the regulator stays on

    DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2);  // PB3=MOSI, PB5=SCK, PB2=CS
    DDRB &= ~(1 << PB4);   // PB4=MISO must be input
}

// === New: External Interrupt Initialization ===
void external_interrupt_init() {
    EICRA |= (1 << ISC01);  // Trigger INT0 on falling edge
    EIMSK |= (1 << INT0);   // Enable INT0 interrupt
    sei();                  // Global interrupt enable
}

// === New: External Interrupt Service Routine ===
ISR(INT0_vect)
{
    pulse_count++;
}

// === Send forced sampling signal on PD4 ===
void send_forced_signal() {
    PORTD |= (1 << PD4);
    _delay_ms(30);
    PORTD &= ~(1 << PD4);
    _delay_ms(500);
}



