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
#include "radio.h"


struct PressureData {
    int t[5];
    int i;
};

#define SLEEP_PERIOD_S 600 // 10 min * 60 seconds = 600s
// Enum definitions
enum state {sleep_period, sampling_period};
enum weather_prediction {rain, norain}; // etc.

// Global variables
volatile enum state current_state = sleep_period;
volatile uint32_t seconds_elapsed = 0;
volatile int changed = 1;
volatile struct PressureData pdata;
volatile int32_t temp = 0;
volatile uint32_t pres = 0;
volatile uint32_t hum = 0;
volatile int dir = 0;
volatile int speed = 0;
volatile int rainfall_sleep_um = 0;
int test = 1;


// Forward declarations
void get_pressure_temp_hum(int32_t *temp, uint32_t *pres, uint32_t *hum);
int get_wind_speed();
int get_rain_depth();
int get_wind_direction();
void start_timer_sleep_mode();
void start_timer_sample_mode();
void send_alert(int alert_num);
void print_status(int pres, int temp, int hum, int wspeed, int rain, int wdir);
char getPressureTrend(struct PressureData pd);
void print_status(int pres, int temp, int rainfall, int wspeed, int rain, int wdir);
void io_pin_init();


int main(void){
    // init all sensors
    TWSR = 0; // Set prescalar for 1
    TWBR = BDIV ; // Set bit rate register

    bme280_init();
    io_pin_init();
    lcd_init();
    weatherSensors_init();
    lcd_clear_screen();
    radio_init();

    start_timer_sleep_mode();

    while(1){
        lcd_write_string("hello");
        _delay_ms(1000);
        radio_debug_print_register(0x2F);
        _delay_ms(1000);

        if (!test){
            lcd_move_cursor(0,0);
            lcd_write_string("test:");
            speed = get_wind_speed();
            dir = get_wind_direction();
            rainfall_sleep_um = get_rain_depth();
            //print_status(1,2,rainfall_sleep_um,speed,dir,6);

            //lcd_clear_screen();
            lcd_move_cursor(0,1);
            char buffy5[32];
            snprintf(buffy5, sizeof(buffy5), "WindSpeed :%2dkph", speed);
            lcd_write_string(buffy5);
            lcd_move_cursor(0,2);
            char buffy6[32];
            snprintf(buffy6, sizeof(buffy6), "Rainfall :%2dmm", rainfall_sleep_um);
            lcd_write_string(buffy6);
            lcd_move_cursor(0,3);
            char buffy4[32];
            snprintf(buffy4, sizeof(buffy4), "Wind Dir:%2ddeg", dir);
            lcd_write_string(buffy4);
            
            changed = 1;
        }
        if (current_state == sleep_period && !test){
            if (seconds_elapsed >= SLEEP_PERIOD_S){ 
                current_state = sampling_period;
                seconds_elapsed = 0;
                current_state = sampling_period;
            }
        }
        else if (current_state == sampling_period && !test){
            lcd_clear_screen();
            lcd_write_string("sampling ...");
            _delay_ms(500);
            lcd_clear_screen();
            get_pressure_temp_hum(&temp, &pres, &hum);
            speed = get_wind_speed();
            rainfall_sleep_um = get_rain_depth();
            dir = get_wind_direction();

            //enum weather_prediction prediction = zambretti_forecast();
            // store or send prediction somewhere

            current_state = sleep_period;
            changed = 1;
            start_timer_sleep_mode();
            
            //radio_send(zambretti()); // return the zambretti prediction as a char array
        }
        // UPDATE THE SCREEN HERE if needed
        if (changed){
            // update LCD here
            //print_status(1,2,rainfall_sleep_um,speed,dir,6);
            //changed = 0;
        }

        //get_pressure_temp_hum();

    }
}

void get_pressure_temp_hum(int32_t *temp, uint32_t *pres, uint32_t *hum){
    uint16_t timeout = 0;
    // float temp, press, hum;

    // bme280_trigger_measurement();
    // while (bme280_is_measuring()) {
    //     _delay_ms(2);
    //     timeout += 2;
    //     if (timeout > 100) {
    //         lcd_clear_screen();
    //         lcd_write_string("Measure Fail");
    //         _delay_ms(500);
    //         changed = 1;
    //         break;  
    //     }
    // }
    // bme280_read_environment(temp, pres, hum);
    // return;

    //return temp pres hum ;
    // STORE TEMP, PRESS, HUM SOMEWHERE
    int fakePressure[5] = { 1,2,3,4,5};

    int i = 0;
    for (i = 0; i<5; i++){
        pdata.t[i] = fakePressure[i];
        char bufs[32];
        snprintf(bufs, sizeof(bufs), "PRES:%d %2d %2d", i, pdata.t[i], fakePressure[i]);
        lcd_write_string(bufs);
        _delay_ms(3000);
    }
}

int get_wind_speed(){
    // "turn on" the wind sensor timer
    // measure for like 10 seconds
    return windSpeed();
}

int get_rain_depth(){
    // fetch global rain depth value
    return rain_um();
}

int get_wind_direction(){
    // get wind vane data
    return windVane();
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
    if (overflow_count >= 3){ // About 1 minute (9)
        seconds_elapsed += 20;
        overflow_count = 0;

        // Update LCD
        lcd_clear_screen();
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Time: %lu Sec", seconds_elapsed / 60);
        lcd_write_string(buffer);
        changed = 1;

        _delay_ms(500);
    }
}


void print_status(int pres, int temp, int rainfall, int wspeed, int wdir, int prediction){
    lcd_clear_screen();
    lcd_move_cursor(0,0);
    char buffy[32];
    snprintf(buffy, sizeof(buffy), "T:%2dC,P:%2dhPa", temp, pres);
    lcd_write_string(buffy);

    lcd_move_cursor(0,1);
    char buffy2[32];
    snprintf(buffy2, sizeof(buffy2), "Wind Speed:%2dmph", wspeed);
    lcd_write_string(buffy2);

    lcd_move_cursor(0,2);
    char buffy4[32];
    snprintf(buffy4, sizeof(buffy4), "Wind Dir:%2ddeg", wdir);
    lcd_write_string(buffy4);

    _delay_ms(500);

    lcd_clear_screen();
    lcd_move_cursor(0,0);
    char buffy5[32];
    snprintf(buffy5, sizeof(buffy5), "Rainfall :%2dmm", rainfall/100);
    lcd_write_string(buffy5);

    lcd_move_cursor(0,1);
    char buffy3[32];
    snprintf(buffy3, sizeof(buffy3), "Forecast: %2d", prediction);
    //lcd_write_string(buffy3);
}

char getPressureTrend(struct PressureData pd){
    // get average
    // get trend
    int diff = pd.t[4] - pd.t[0];

    if (diff > 5) {         // threshold to avoid noise
        return 'r';           // rising
    } else if (diff < -5) {
        return 'f';          // falling
    } else {
        return 's';           // stable
    }

    _delay_ms(500);
}

void io_pin_init(){
    // init everything as outputs ( PB0-PB5, PB7, PC0-PC5, PD0-PD7 )
    DDRD = 0xFF;  // 0xFF = 1111 1111 (all bits set to 1)
    DDRC = (1 << PC5 | 1 << PC4 | 1 << PC0); // 0011 0001 PC3,2,1 to input
    DDRB = 0xBF; // 1011 1111
    // don't work : PB3, PC0, PD

    PORTD &= ~(0xFF);
    PORTC &= ~(0x3F);
    PORTB &= ~(0xBF);

    // Ensure PD2 (INT0/DIO0) remains input with pull-up
    DDRD &= ~(1 << PD2);
    PORTD |=  (1 << PD2);
}