#include <msp430.h> 
#include <stdio.h>
#include "gearmotor.h"
#include "debug.h"

//-- Types of tests
#define MOVE_TIME_T 1

//-- CONTROL
#define TEST_SEL MOVE_TIME_T
#define VERBOSE 1
#define V_Print(str) if(VERBOSE) {putString(str);}

int GearMotorOn = 0;

void test_motorMovementTimeout(void);

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_gearmotor();
    init_UART_DBG();

    __enable_interrupt();

    switch(TEST_SEL) {
    case MOVE_TIME_T:
        test_motorMovementTimeout();
        break;
    }

    V_Print("DONE\n\r")
    for(;;);
}


//-- TESTS --

void test_motorMovementTimeout(void) {

    //--Start the gearMotor with 1s timeout Forward
    startGearMotor(1, SLOW, 0.2);

    while(GearMotorOn);

    //--Start the gearMotor with 1s timeout Backward
    startGearMotor(0, SLOW, 0.2);

    while(GearMotorOn);

    __delay_cycles(35000);

    //--Start the gearMotor with 1s timeout Forward
    startGearMotor(1, MEDIUM, 0.2);

    while(GearMotorOn);

    //--Start the gearMotor with 1s timeout Backward
    startGearMotor(0, MEDIUM, 0.2);

    while(GearMotorOn);

    __delay_cycles(35000);

    //--Start the gearMotor with 1s timeout Forward
    startGearMotor(1, FAST, 0.1);

    while(GearMotorOn);

    //--Start the gearMotor with 0.5s timeout Backward
    startGearMotor(0, FAST, 0.1);

    while(GearMotorOn);

}
