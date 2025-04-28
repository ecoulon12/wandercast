/********************************************
*
*  Name: Erin Coulon, Vic Nunez, Drew Uramoto
*  Section: EE459L
*  Assignment: Wandercast
*
********************************************/

#define NULL 
#define FOSC 9830400 // Clock frequency = Oscillator freq.
#define FOSC1 7372800 
#define BDIV ( (FOSC1 / 200000) - 16) / 2 + 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"
#include "bme280.h"
#include "tiny_bme280.h"
#include "weatherSensors.h"
#include "zambretti.h"

#define SLEEP_PERIOD_S 600 // 10 min * 60 seconds = 600s

// Enum definitions
enum state {sleep_period, sampling_period};
enum weather_prediction {rain, norain}; // etc.

// Global variables
volatile enum state current_state = sleep_period;
volatile uint32_t seconds_elapsed = 0;


// Forward declarations
void get_pressure_temp_hum();
void get_wind_speed();
void get_rain_depth();
void get_wind_direction();
void start_timer_sleep_mode();
void start_timer_sample_mode();
void send_alert(int alert_num);
void print_status(int pres, int temp, int hum, int wspeed, int rain, int wdir);

int main(void){
    // init all sensors
    TWSR = 0; // Set prescalar for 1
    TWBR = BDIV ; // Set bit rate register

    bme280_init();
    lcd_init();
    weatherSensors_init();
    lcd_clear_screen();

    start_timer_sleep_mode();

    while(1){
        if (current_state == sleep_period){
            if (seconds_elapsed >= SLEEP_PERIOD_S){ 
                current_state = sampling_period;
                seconds_elapsed = 0;
                current_state = sampling_period;
            }
        }
        else if (current_state == sampling_period){
            lcd_write_string("sampling ...");
            _delay_ms(2000);
            get_pressure_temp_hum();
            get_wind_speed();
            get_rain_depth();
            get_wind_direction();

            //enum weather_prediction prediction = zambretti_forecast();
            // store or send prediction somewhere

            current_state = sleep_period;
            start_timer_sleep_mode();
        }
        // UPDATE THE SCREEN HERE if needed
        if (changed){
            // update LCD here
        }

    }
}

void get_pressure_temp_hum(){
    // uint16_t timeout = 0;
    // float temp, press, hum;

    // bme280_trigger_measurement();
    // while (bme280_is_measuring()) {
    //     _delay_ms(2);
    //     timeout += 2;
    //     if (timeout > 100) {
    //         lcd_clear_screen();
    //         lcd_write_string("Measure Fail");
    //         break;  
    //     }
    // }
    // bme280_read_environment(&temp, &press, &hum);
    //return temp pres hum ;
    // STORE TEMP, PRESS, HUM SOMEWHERE
}

void get_wind_speed(){
    // "turn on" the wind sensor timer
    // measure for like 10 seconds
}

void get_rain_depth(){
    // fetch global rain depth value
    // return rain_depth();
}

void get_wind_direction(){
    // get wind vane data
    //return windVane();
}

void start_timer_sleep_mode(){
    TCCR1A = 0;
    TCCR1B = 0;

    // Set prescaler clk/1024
    TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);

    // Enable Timer1 overflow interrupt
    TIMSK1 |= (1 << TOIE1);

    // Initialize counter
    TCNT1 = 0;

    // Enable global interrupts
    sei();
}

void start_timer_sample_mode(){
    // start timer for measuring wind speed
}

void send_alert(int alert_num){
    // send radio alerts
}

// TIMER1 overflow ISR
ISR(TIMER1_OVF_vect){
    static uint16_t overflow_count = 0;

    overflow_count++;

    // Each overflow at clk/1024 is about 6.827 seconds
    if (overflow_count >= 9){ // About 1 minute
        seconds_elapsed += 60;
        overflow_count = 0;

        // Update LCD
        lcd_clear_screen();
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Time: %lu min", seconds_elapsed / 60);
        lcd_write_string(buffer);
        changed = 1;

        _delay_ms(2000;)
    }
}


void print_status(int pres, int temp, int hum, int wspeed, int wdir, int prediction){
    lcd_clear_screen();
    lcd_move_cursor();
    char buffy[16];
    snprintf(buffy, sizeof(buffy), "T:%2dC,P:%2dhPa,H:%2d", temp, pres, hum);
    lcd_write_string(buffy);

    lcd_move_cursor();
    char buffy2[16];
    snprintf(buffy2, sizeof(buffy2), "Speed:%2dmph,Dir:%2ddeg", wspeed, wdir);
    lcd_write_string(buffy2);

    lcd_move_cursor();
    char buffy3[16];
    snprintf(buffy3, sizeof(buffy3), "Forecast: %2d", prediction);
    lcd_write_string(buffy3);
}