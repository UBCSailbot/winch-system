#include <msp430.h> 
#include <stdio.h>
#include "gearmotor.h"
#include "debug.h"
#include "spi.h"

//-- Types of tests
#define MOVE_TIME_T 1
#define SPI_DATA_T 2

//-- CONTROL
#define TEST_SEL MOVE_TIME_T
#define VERBOSE 1
#define V_PRINT(str) if(VERBOSE) {putString(str);}

int GearMotorOn = 0;

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

    startGearMotor(1, SLOW, 0.2);
    while(GearMotorOn);

    __delay_cycles(35000);

    startGearMotor(0, SLOW, 0.2);
    while(GearMotorOn);

    __delay_cycles(35000);

    startGearMotor(1, MEDIUM, 0.2);
    while(GearMotorOn);

    __delay_cycles(35000);

    startGearMotor(0, MEDIUM, 0.2);
    while(GearMotorOn);

    __delay_cycles(35000);

    startGearMotor(1, FAST, 0.1);
    while(GearMotorOn);

    __delay_cycles(35000);

    startGearMotor(0, FAST, 0.1);
    while(GearMotorOn);

    __delay_cycles(35000);
}

void test_receiveSPI(void) {

    int timeout[3] = {0.2, 0.2, 0.1};
    int speeds[3] =  {SLOW, MEDIUM, FAST};
    char* speeds_str[3] = {"SLOW", "MEDIUM", "FAST"};
    int index = 0;
    int pawl_left, pawl_right, cam;
    char str[50] = "";

    while (1) {

        //-- Move to the right
        startGearMotor(1, speeds[index], timeout[index]);
        while(GearMotorOn);

        receive_hallsensors(NULL, NULL, &pawl_right);

        sprintf(str, "[%s] Pawl right: %d\r\n\0", speeds_str[index], pawl_right);
        V_PRINT(str);

        __delay_cycles(35000);

        //-- Move to the left
        startGearMotor(0, speeds[index], timeout[index]);
        while(GearMotorOn);

        receive_hallsensors(&pawl_left, NULL, NULL);

        sprintf(str, "[%s] Pawl left: %d\r\n\0", speeds_str[index], pawl_left);
        V_PRINT(str);

        __delay_cycles(35000);

        //-- Move to the center
        startGearMotor(1, speeds[index], timeout[index]/2);

        receive_hallsensors(NULL, &cam, NULL);

        sprintf(str, "[%s] CAM: %d\r\n\0", speeds_str[index], cam);
        V_PRINT(str);

        __delay_cycles(35000);

        //-- Get back to the left
        startGearMotor(0, speeds[index], timeout[index]/2);

        index++;
        index %= 3;
    }
}
