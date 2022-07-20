#include <msp430.h> 
#include <stdio.h>
#include "gearmotor.h"
#include "debug.h"
#include "spi.h"
#include "uart.h"

//-- Types of tests
enum test_type
{
    MOVE_TIME_T,
    MOVE_SPI_DATA_T
};

//-- SELECT TEST TO BE PERFORMED HERE
enum test_type test_sel = MOVE_SPI_DATA_T;


#define DELAY 3500000
#define SPEED_SEL 1     //-- Slow: 0 Medium: 1 Fast: 2


void test_motorMovementTimeout(void);
void test_receiveSPI(void);

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_gearmotor();
    init_uart();
    init_spi();

    __enable_interrupt();

    switch(test_sel) {
    case MOVE_TIME_T:
        test_motorMovementTimeout();
        break;
    case MOVE_SPI_DATA_T:
        test_receiveSPI();
        break;
    }

    V_PRINTF("DONE\n\r")
    for(;;);
}


//-- TESTS --

void test_motorMovementTimeout(void) {

    startGearMotor(1, SLOW, 200);
    while(isGearMotorOn());

    __delay_cycles(DELAY);

    startGearMotor(0, SLOW, 200);
    while(isGearMotorOn());

    __delay_cycles(DELAY);

    startGearMotor(1, MEDIUM, 200);
    while(isGearMotorOn());

    __delay_cycles(DELAY);

    startGearMotor(0, MEDIUM, 200);
    while(isGearMotorOn());

    __delay_cycles(DELAY);

    startGearMotor(1, FAST, 100);
    while(isGearMotorOn());

    __delay_cycles(DELAY);

    startGearMotor(0, FAST, 100);
    while(isGearMotorOn());

    __delay_cycles(DELAY);
}

void test_receiveSPI(void) {

    int timeout[3] = {300, 100, 50};
    int speeds[3] =  {SLOW, MEDIUM, FAST};
    char* speeds_str[3] = {"SLOW\0", "MEDIUM\0", "FAST\0"};
    unsigned int index = SPEED_SEL;
    unsigned int pawl_left, pawl_right, cam;

    V_PRINTF("test_receiveSPI");

    while (1) {

        //-- Move to the right
        startGearMotor(1, speeds[index], timeout[index]);
        while(isGearMotorOn());

        receive_hallsensors(&pawl_left, &cam, &pawl_right);

        V_PRINTF("[%s] RIGHT: Pawl left: %x, CAM: %d, Pawl_right: %x \r\n\0", speeds_str[index], pawl_left, cam, pawl_right);

        __delay_cycles(DELAY);

        //-- Move to the left
        startGearMotor(0, speeds[index], timeout[index]);
        while(isGearMotorOn());

        receive_hallsensors(&pawl_left, &cam, &pawl_right);

        V_PRINTF("[%s] LEFT: Pawl left: %x, CAM: %d, Pawl_right: %x\r\n\0", speeds_str[index], pawl_left, cam, pawl_right);

        __delay_cycles(DELAY);

        //-- Move to the center
        startGearMotor(1, speeds[index], timeout[index]/(2*(index+1)));
        while(isGearMotorOn());

        receive_hallsensors(&pawl_left, &cam, &pawl_right);

        V_PRINTF("[%s] CENTER: Pawl left: %x, CAM: %d, Pawl_right: %x\r\n\0", speeds_str[index], pawl_left, cam, pawl_right);

        __delay_cycles(DELAY);
    }
}
