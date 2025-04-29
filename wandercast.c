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
    int currPres;
};

#define SLEEP_PERIOD_S 60 // 10 min * 60 seconds = 600s
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
int test = 0;
volatile char ptrend;
int real = 0;
volatile uint8_t forced_sample_requested = 0;



// Forward declarations
void get_pressure_temp_hum(int32_t *temp, uint32_t *pres, uint32_t *hum);
int get_wind_speed();
int get_rain_depth();
int get_wind_direction();
void start_timer_sleep_mode();
void start_timer_sample_mode();
void send_alert(int alert_num);
char getPressureTrend();
void print_status(int pres, int temp, int rainfall, int wspeed, int rain, const char* pred);
void io_pin_init();
void set_output_mode_pd7();
void pin_interrupt_init();
// void pin_interrupt_init();
void set_input_mode_pb0();
void send_forecast_serial(const char* pred);
int pred_to_pulses(const char* pred);
void disable_pb0();

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
    pin_interrupt_init();

    start_timer_sleep_mode();

    while(1){
        if (test){

            lcd_clear_screen();
            lcd_write_string("sampling ...");
            _delay_ms(500);
            lcd_clear_screen();
            
            // Fetch and sample for sensor data
            get_pressure_temp_hum(&temp, &pres, &hum);
            ptrend = getPressureTrend();
            speed = get_wind_speed();
            rainfall_sleep_um = get_rain_depth();
            dir = get_wind_direction();

            current_state = sleep_period;
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
            
            // Fetch and sample for sensor data
            get_pressure_temp_hum(&temp, &pres, &hum);
            ptrend = getPressureTrend();
            speed = get_wind_speed();
            rainfall_sleep_um = get_rain_depth();
            dir = get_wind_direction();

            current_state = sleep_period;
            changed = 1;
            start_timer_sleep_mode();

            lcd_clear_screen();
            lcd_write_string("sending forecast ...");
            _delay_ms(500);
            //radio_send(zambretti()); // return the zambretti prediction as a char array
            send_forecast_serial(zambretti_forecast(pdata.currPres,ptrend, dir, speed));
            lcd_clear_screen();
        }
        // UPDATE THE SCREEN HERE if needed
        if (changed){
            // update LCD here
            //print_status(ptrend, pdata.currPres, rainfall_sleep_um , speed ,dir , zambretti_forecast(pdata.currPres,ptrend, dir, speed));
            //changed = 0;
        }

    }
}

void get_pressure_temp_hum(int32_t *temp, uint32_t *pres, uint32_t *hum){
    uint16_t timeout = 0;
    // float temp, press, hum;

    if (real){
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
    } else{
        int fakePressure[5] = { 1011,1011,1011,1011,900};

        int i = 0;
        pdata.currPres = fakePressure[4];
        for (i = 0; i<5; i++){
            pdata.t[i] = fakePressure[i];
        }
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
        snprintf(buffer, sizeof(buffer), "Time: %lu Sec", seconds_elapsed);
        lcd_write_string(buffer);
        changed = 1;

        _delay_ms(500);
    }
}


void print_status(int temp, int pres, int rainfall, int wspeed, int wdir,  const char* pred){
    lcd_clear_screen();
    lcd_move_cursor(0,0);
    char buffy[32];
    snprintf(buffy, sizeof(buffy), "T:%c P:%4dhPa", temp, pres);
    lcd_write_string(buffy);

    lcd_move_cursor(0,1);
    char buffy2[32];
    snprintf(buffy2, sizeof(buffy2), "Wind Speed:%2dmph", wspeed);
    lcd_write_string(buffy2);

    lcd_move_cursor(0,2);
    char buffy4[32];
    snprintf(buffy4, sizeof(buffy4), "Wind Dir:%2ddeg", wdir);
    lcd_write_string(buffy4);

    _delay_ms(1000);

    lcd_clear_screen();
    lcd_move_cursor(0,0);
    char buffy5[32];
    snprintf(buffy5, sizeof(buffy5), "Rainfall :%2dmm", rainfall/100);
    lcd_write_string(buffy5);

    lcd_move_cursor(0,1);
    lcd_write_string("Forecast: ");

    lcd_move_cursor(0,2);
    char buffy3[32];
    snprintf(buffy3, sizeof(buffy3), "%s", pred);
    lcd_write_string(buffy3);

    _delay_ms(1000);
}

char getPressureTrend(){
    // get average
    // get trend
    int diff = pdata.t[4] - pdata.t[0];

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

    DDRB &= ~(1 << PB0);   // PB0 as input
    PORTB |= (1 << PB0);   // Enable pull-up resistor on PB0

}

void pin_interrupt_init() {
    // Set PB0 as input and pull-up
    // DDRB &= ~(1 << PB0);
    // PORTB |= (1 << PB0);
    // should this be pulled up? it is set as an input

    // Enable Pin Change Interrupt for PB[7:0] (PCIE0)
    PCICR |= (1 << PCIE0);      // Enable PCINT7..0 group (Port B)
    PCMSK0 |= (1 << PCINT0);    // Enable PB0 (PCINT0)
}

void send_forecast_serial(const char* pred) {
    int pulses = pred_to_pulses(pred);
    set_output_mode_pd7();  // PB0 instead of PD4
    disable_pb0();

    uint8_t i;
    for (i = 0; i < pulses; i++) {
        PORTD |= (1 << PD7);    // Set PB0 high
        _delay_ms(20);          // Pulse HIGH duration
        PORTD &= ~(1 << PD7);   // Set PB0 low
        _delay_ms(500);         // Time between pulses
    }

    set_input_mode_pb0();  // Back to input mode
}


void set_output_mode_pd7() {
    // Disable Pin Change Interrupt for PB0
    // PCICR &= ~(1 << PCIE0);    // Disable Pin Change Interrupt for Port B group
    // PCMSK0 &= ~(1 << PCINT0);  // Mask off PB0

    // Set PB0 as output
    DDRD |= (1 << PD7);

    // Drive low initially
    PORTD &= ~(1 << PD7);
}

void disable_pb0() {
    // Disable Pin Change Interrupt for PB0
    PCICR &= ~(1 << PCIE0);    // Disable Pin Change Interrupt for Port B group
    PCMSK0 &= ~(1 << PCINT0);  // Mask off PB0

    // // Set PB0 as output
    // DDRD |= (1 << PB0);

    // // Drive low initially
    // PORTD &= ~(1 << PB0);
    DDRB   |=  (1<<PB0);       // make PB0 an output
    PORTB  &= ~(1<<PB0);       // drive it low
}


void set_input_mode_pb0() {
    // Set PB0 back to input
    DDRB &= ~(1 << PB0);

    // Enable pull-up resistor
    //PORTB |= (1 << PB0);

    // Re-enable Pin Change Interrupt for PB0
    PCICR |= (1 << PCIE0);      // Enable PCINT0 group (PB[7:0])
    PCMSK0 |= (1 << PCINT0);    // Enable interrupt on PB0
}

int pred_to_pulses(const char* pred){
    if (strcmp(pred, "Fine Weather") == 0) {
        return 1;
    } else if (strcmp(pred, "Fair Weather") == 0) {
        return 2;
    } else if (strcmp(pred, "Unsettled, Improving") == 0) {
        return 3;
    } else if (strcmp(pred, "Fair Weather (falling)") == 0) {
        return 4;
    } else if (strcmp(pred, "Unsettled (falling)") == 0) {
        return 5;
    } else if (strcmp(pred, "Fair, then Showers") == 0) {
        return 6;
    } else if (strcmp(pred, "Showers") == 0) {
        return 7;
    } else if (strcmp(pred, "Rain") == 0) {
        return 8;
    } else {
        return 9;  // Unknown forecast → 9 pulses to indicate "error" or "unknown"
    }
}

ISR(PCINT0_vect) {
    // Check if PB0 went low
    lcd_clear_screen();
    lcd_write_string("PCINT!");
    _delay_ms(1000);
    // if (!(PINB & (1 << PB0))) {
    //     //forced_sample_requested = 1;
    //     seconds_elapsed = 0;
    //     current_state = sampling_period;
    // }

    seconds_elapsed = 0;
    current_state = sampling_period;
}

