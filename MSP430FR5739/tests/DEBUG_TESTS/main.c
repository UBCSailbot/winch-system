#include <msp430.h>
#include "debug.h"

//-- Types of tests
enum test_type
{
    MMOTOR_ON
};

//-- SELECT TEST TO BE PERFORMED HERE
enum test_type test_sel = MMOTOR_ON;

#define ON_MOTOR BIT3

void test_mmotor_on(void);

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    __enable_interrupt();

    P3DIR |= ON_MOTOR;
    P3OUT |= ON_MOTOR;

    switch(test_sel) {
    case MMOTOR_ON:
        test_mmotor_on();
        break;
    }

    for(;;);
}


//-- TESTS --
void test_mmotor_on()
{
    V_PRINTF("Main motor on\n\r")

    //-- Enable port that is connected to input 4 on the motor controller
    P3DIR |= ON_MOTOR;
    P3OUT |= ON_MOTOR;
}
