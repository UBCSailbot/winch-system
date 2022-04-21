/*
 * motor.c
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include "motor.h"

int motor_increment;

/**
 * P2.2 DIR: 1 forward and 0 backward TODO: confirm
 *
 * P3.4 STEP: Uses TB1 CCR1 capture register
 */
void init_Main_Motor(void) {

    motor_increment = 0;

    //-- Init DIR port to output
    P2DIR |= DIR;
    P2OUT &= ~DIR;

    //-- Initialize PWM on STEP port
    //-- P3.4 STEP TB1.1
    P3DIR |= STEP;
    P3SEL1 &= ~STEP;
    P3SEL0 |= STEP;
    P3OUT &= ~STEP;

    //-- TB1 reg 1 timer setup
    TB1CCR0 = UPPER_COUNT - 1;
    TB1CCR1 = MID_COUNT;
    TB1CCTL1 |= OUTMOD_2;           // Toggle set mode
    TB1CTL |= TBSSEL_2;             // SMCLK 1 Mhz

}

/**
 * Rotates the main motor in the direction indicated by the
 * global state
 *
 * Returns -1 when error and 0 otherwise
 */
int startMainMotor(int increment) {
    //-- set direction

    switch(cur_direction) {
    case CLOCKWISE:
        P2OUT |= DIR;
        break;

    case ANTICLOCKWISE:
        P2OUT &= ~DIR;
        break;

    default:
        return -1;  // Action not completed
    }

    motor_increment = increment;

    TB1CTL |= TBCLR; // Clear timer count
    TB1CTL |= MC_1;   //-- Count up mode

    TB1CCTL0 |= CCIE;   // Enable interrupts on reg 0

    return 0;
}


#pragma vector = TIMER1_B0_VECTOR;
__interrupt void TIMER1_B0_ISR (void) {
    //-- Decrement no. of steps, if zero stop motor


    if (motor_increment <= 0) {
        TB1CCTL0 &= ~CCIE;  // Disable interrupts
        TB1CTL &= ~MC_1;    // Hault PWM timer
        TB1CTL |= TBCLR;    // Clear timer count
        motor_increment = 0;
    } else {
        motor_increment--;
    }

    TB1CCTL0 &= ~CCIFG;     // Clear interrupt flag
}


