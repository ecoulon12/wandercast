# wandercast

WORKFLOW GUIDE:
Since we are not using branches, we need to sycnhronize verbally, so please text in the GC if you are changing any files
Also, before editing anything, rememeber to pull for any new changes so we don't have to handle merge conflicts

File list:
ee459.c : main loop, can edit to test things

I2C: [DO NOT EDIT]

    i2c.c : function definitions for i2c (written by weber) -- DO NOT EDIT
        - i2c_init(bdiv) -- initialize things for i2c
        - i2c_io( params ) -- perform i2c read/write transactions

    i2c.h : header file for i2c -- DO NOT EDIT
        - contains i2c function prototypes

BAROMETER : (most of these are unfinished)

    bme280.c : function definitions for barometer
        - bme280_init(void) -- initialize barometer chip based on recommended settings in the datasheet (for weather monitoring applications)
        - bme280_get_id() -- prints out the ID of the chip (primarily used to test if chip is functioning, upon success, should print 0x60)
        - bme280_read_reg(const char *str) -- 
        - bme280_get_status() -- returns 1 if the sensor is currently measuring, 0 if not

    bme280.h : header file for barometer sensor
        - contains barometer function prototypes, variable definitions (i2c address, register addresses), debug mode ctrls

LCD : (current functions are finished, but we can add more as we need)

    lcd.c : function definitions for the lcd screen
        - lcd_init(void) -- initialize lcd 
        - lcd_write_string(const char *str) -- write a string to the lcd screen
        - lcd_clear_screen(void) -- clears the lcd screen

    lcd.h : header file for lcd screen
        -   Contains lcd function prototypes, variable definitions (addresses), debug mode ctrls


HOW TO ADD NEW FUNCTIONS TO AN EXISTING .C FILE:
1. Add the function prototype to the corresponding .h file
2. Implement function in the .c file

HOW TO ADD NEW FUNCTIONS IN A NEW .C FILE:
(to expain, lets say the new file is called sensor.c)
1. Create new sensor.c and sensor.h file 
2. In Makefile, add sensor.o into line 4 ("OBJECTS" line)
3. Go to h_file_template.txt, copy .h file contents into sensor.h, add in prototypes for functions and any variable definitions
4. in h_file_template.txt, copy .c file contents into sensor.c, and start writing function implementations

HOW TO 
1. Clone the repo
- go to the github wandercast page 
- press the drop down arrow on the green "Code" button
- copy the SSH key
- in your terminal, go to where you want to add the wandercast folder
- use the command "git clone {SSH KEY}"

2. Editing files
- you can do this in any 

3. Running Code
- plug in 
- if you suspect compilation errors, just use the "make" command