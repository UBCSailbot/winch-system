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
    startGearMotor(1, FAST, 1);
    V_Print("GEAR MOTOR STARTED - FAST\n\r")

    while(GearMotorOn);

    //--Start the gearMotor with 1s timeout Backward
    startGearMotor(0, MEDIUM, 1);
    V_Print("GEAR MOTOR STARTED - MEDIUM\n\r")

    while(GearMotorOn);

    //--Start the gearMotor with 1s timeout Forward
    startGearMotor(1, SLOW, 1);
    V_Print("GEAR MOTOR STARTED - SLOW\n\r")

    while(GearMotorOn);

    //--Start the gearMotor with 0.5s timeout Backward
    startGearMotor(0, MEDIUM, 0.5);
    V_Print("GEAR MOTOR STARTED - MEDIUM\n\r")

    while(GearMotorOn);

}
