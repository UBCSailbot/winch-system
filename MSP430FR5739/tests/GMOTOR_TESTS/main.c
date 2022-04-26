#include <msp430.h> 
#include <stdio.h>
#include "gearmotor.h"
#include "debug.h"
#include "spi.h"

//-- Types of tests
#define MOVE_TIME_T 1
#define SPI_DATA_T 2

//-- CONTROL
#define TEST_SEL SPI_DATA_T
#define VERBOSE 1
#define WAIT 3500000
#define SPEED_SEL 1     //-- Slow: 0 Medium: 1 Fast: 2

#define V_PRINT(str) if(VERBOSE) {putString(str);}


void test_motorMovementTimeout(void);
void test_receiveSPI(void);

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_gearmotor();
    init_UART_DBG();
    init_spi();

    __enable_interrupt();

    switch(TEST_SEL) {
    case MOVE_TIME_T:
        test_motorMovementTimeout();
        break;
    case SPI_DATA_T:
        test_receiveSPI();
        break;
    }

    V_PRINT("DONE\n\r")
    for(;;);
}


//-- TESTS --

void test_motorMovementTimeout(void) {

    startGearMotor(1, SLOW, 200);
    while(isGearMotorOn());

    __delay_cycles(WAIT);

    startGearMotor(0, SLOW, 200);
    while(isGearMotorOn());

    __delay_cycles(WAIT);

    startGearMotor(1, MEDIUM, 200);
    while(isGearMotorOn());

    __delay_cycles(WAIT);

    startGearMotor(0, MEDIUM, 200);
    while(isGearMotorOn());

    __delay_cycles(WAIT);

    startGearMotor(1, FAST, 100);
    while(isGearMotorOn());

    __delay_cycles(WAIT);

    startGearMotor(0, FAST, 100);
    while(isGearMotorOn());

    __delay_cycles(WAIT);
}

void test_receiveSPI(void) {

    int timeout[3] = {300, 100, 50};
    int speeds[3] =  {SLOW, MEDIUM, FAST};
    char* speeds_str[3] = {"SLOW\0", "MEDIUM\0", "FAST\0"};
    unsigned int index = SPEED_SEL;
    int pawl_left, pawl_right, cam;
    char str[300] = "";

    V_PRINT("test_receiveSPI");

    while (1) {

        //-- Move to the right
        startGearMotor(1, speeds[index], timeout[index]);
        while(isGearMotorOn());

        receive_hallsensors(&pawl_left, &cam, &pawl_right);

        sprintf(str, "[%s] RIGHT: Pawl left: %d, CAM: %d, Pawl_right: %d \r\n\0", speeds_str[index], pawl_left, cam, pawl_right);
        V_PRINT(str);

        __delay_cycles(WAIT);

        //-- Move to the left
        startGearMotor(0, speeds[index], timeout[index]);
        while(isGearMotorOn());

        receive_hallsensors(&pawl_left, &cam, &pawl_right);

        sprintf(str, "[%s] LEFT: Pawl left: %d, CAM: %d, Pawl_right: %d\r\n\0", speeds_str[index], pawl_left, cam, pawl_right);
        V_PRINT(str);

        __delay_cycles(WAIT);

        //-- Move to the center
        startGearMotor(1, speeds[index], timeout[index]/(2*(index+1)));
        while(isGearMotorOn());

        receive_hallsensors(&pawl_left, &cam, &pawl_right);

        sprintf(str, "[%s] CENTER: Pawl left: %d, CAM: %d, Pawl_right: %d\r\n\0", speeds_str[index], pawl_left, cam, pawl_right);
        V_PRINT(str);

        __delay_cycles(WAIT);
    }
}
