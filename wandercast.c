/********************************************
*
*  Name: Erin Coulon, Vic Nunez, Drew Uramoto
*  Section: EE459L
*  Assignment: Wandercast
*
********************************************/



#define NULL 
# define FOSC 9830400 // Clock frequency = Oscillator freq .
# define FOSC1 7372800 
# define BDIV ( FOSC1 / 200000 - 16) / 2 + 1

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"
#include "bme280.h"
#include "tiny_bme280.h"
#include "weatherSensors.h"
#include "zambretti.h"

enum state {sleep_period, sampling_period}
enum weather_prediction {rain norain etc}
// data structure (array) to store 

int main(void){
    // init all sensors
    // init lcd
    // init anything else

    while(1){
        //have a sampling period every hour
            // sampling period = take 6 readings total
        // Average the 6 readings
            // readings : 
            // force reading of bme280

            // wind vane
            // anemometer
            // rain sensor -- always on, ticks every x inches of rain (PCI)
        // store readings 
        // update Zambretti prediction
        // send out any necessary updates

        // ALSO -- add a button to FORCE a sampling period (basically resets everything and changes state to sampling_period)
            // PCI needed

        if (state == sleep_period){

        }else if (state == sampling_period){
            get_pressure_temp_hum();
            get_wind_speed();
            get_rain_depth();
            get_wind_direction();

            //store all this stuff somewhere
            weather_prediction = zambretti_forecast();
            
        }
    }
}

void get_pressure_temp_hum(){
    bme280_trigger_measurement();
    while (bme280_is_measuring()) { // here for read errors
        _delay_ms(2);
        timeout += 2;
        if (timeout > 100) {
            lcd_clear_screen();
            lcd_write_string("Measure Fail");
            break;  
        }
    }
    bme280_read_environment(&temp, &press, &hum);
    // STORE SOMEWHERE
}

void get_wind_speed(){
    //"turn on" the wind sensor timer
    // measure for like 10 seconds
}

void get_rain_depth(){
    // basically just go fetch the global value that is keeping track of rain ticks
}

void get_wind_direction(){
    // use weatherSensors.c api to get wind_vane data
}

// Timers :
// Timer 1 
    // timer for an hour in sleep mode
    // if in read wind_speed mode, then do that stuff


void send_alert(int alert_num){
    // whatever radio code to send here, taking into account the 
}

