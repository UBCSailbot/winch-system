/*
 * gearmotor.c
 *
 *  Created on: Apr. 4, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include "gearmotor.h"

//-- State of gearmotor
volatile int GearMotorOn = 0;

void init_gearmotor(void) {

    //-- PJ.3 NFAULT Input
    PJDIR &= ~NFAULT;
    PJREN &= ~NFAULT;    // Pull-up resistor disabled - Pull-up already on the PCB

    //-- PJ.2 MODE Output
    PJDIR |= MODE;
    PJOUT &= ~MODE;

    //-- PJ.1 PHASE Output
    PJDIR |= PHASE;
    PJOUT &= ~PHASE;

    //-- PJ.0 NSLEEP Output
    PJDIR |= NSLEEP;
    PJOUT &= ~NSLEEP;

    //-- P1.4 ENABLE Output
    P1DIR |= ENABLE;
    P1OUT &= ~ENABLE;


    //-- Initialize PWM on the Enable bit --

    //-- Port configuration --

    //-- P1.4 ENABLE TB0.1 Output
    P1DIR |= ENABLE;
    P1SEL1 &= ~BIT4;
    P1SEL0 |= BIT4;
    P1OUT |= ENABLE;


    //-- Refer to 12.2.5.1.1
    //-- CCR0 for upper trapezoidal limit (lowers output), determines PWM frequency
    //-- CCR1 for lower trapezoidal limit (raises output), CCR1 < CCR0

    TB0CCR0 = D_PWM_UPPERC; //-- Timer_B Main timer 700Hz, TB0CCR0 is buffered to TB0CL0
    TB0CCTL1 |= OUTMOD_2;   //-- OUTMOD - Toggle/reset, equivalent to PWM
    TB0CTL |= TBSSEL_2;     //-- Clock SMCLK (1 Mhz)


    //-- Initialize timeout timer --


    //-- Timer_A Main timer 1s
    TA1CCR0 = D_TIMEOUTC;
    //-- ACLK (10 khz)
    TA1CTL |= TASSEL_1;


}

/*
 * @brief Starts motor with given direction, speed, and timeout
 *  int forward --> 0 for reverse (anti-clockwise, towards left pawl)
 *                  1 for forward (clockwise, towards right pawl)
 *  int speed   --> PWM duty cycle, input from 1 to 1427 (D_PWM_UPPERC-1)
 *                  1427 is 1% duty cycle, 1 is 99% duty cycle
 *                  Formula is 1427*(1-duty_decimal)
 *  int timeout --> Timeout for max duration of motor run after startGearMotor
 *                  Resets to new timeout when startGearMotor is run again
 *                  Input in millisecond, example 500 = 500 milliseconds
 *                  Max input is 6553 (6.553s)
 */
void startGearMotor(int forward, int speed, int timeout) {

    PJOUT &= ~MODE;     //-- FAST decay mode

    //-- Direction --
    if (forward) PJOUT |= PHASE;
    else PJOUT &= ~PHASE;

    PJOUT |= NSLEEP;    //-- IC active

    //-- PWM control --
    TB0CCR1 = speed;
    TB0CTL |= MC_1;     //-- Count up mode

    if ( timeout != NO_TIMEOUT ) {
        //-- Timeout timer --
        TA1CCR0 = (timeout * 10);
        TA1CTL |= MC_1;     //-- Count up mode
        TA1CTL |= TACLR;    //-- Clear timer count

        TA1CCTL0 |= CCIE;   //-- Enable interrupts
    }


    //-- Set motor state to running
    GearMotorOn = 1;
}

void stopGearMotor(void) {

    //-- MC - idle, Disable PWM
    TB0CTL &= ~MC_1;

    //-- Put into sleep
    PJOUT &= ~NSLEEP;

    //-- Disable count up mode (No timeout)
    TA1CTL &= ~MC_1;

    GearMotorOn = 0;

    //-- Disable interrupts
    TA1CCTL0 &= ~CCIE;

    TA1CTL |= TACLR;
}

int isGearMotorOn(void) {
    return GearMotorOn;
}

#pragma vector = TIMER1_A0_VECTOR;
__interrupt void TIMER1_A0_ISR (void) {

    //-- Clear flag
    TA1CCTL0 &= ~CCIFG;

    stopGearMotor();
}

