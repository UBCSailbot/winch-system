/*
 * gearmotor.c
 *
 *  Created on: Apr. 4, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include "gearmotor.h"


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

    //-- Timer_B Main timer 0.03125s
    TB0CCR0 = D_PWM_UPPERC;
    //-- OUTMOD - Toggle/reset
    TB0CCTL1 |= OUTMOD_2;
    //-- Clock ACLK (32.768 khz)
    TB0CTL |= TBSSEL_1;


    //-- Initialize timeout timer --

    //-- Timer_A Main timer 1s
    TA1CCR0 = D_TIMEOUTC;
    //-- ACLK (32.768 khz) / 2
    TA1CTL |= TASSEL_1;


}

void startGearMotor(int forward, int speed, float timeout) {

    PJOUT &= ~MODE;     //-- FAST decay mode

    //-- Direction --
    if (forward) PJOUT |= PHASE;
    else PJOUT &= ~PHASE;

    PJOUT |= NSLEEP;    //-- IC active

    //-- PWM control --
    TB0CCR1 = speed;
    TB0CTL |= MC_1;     //-- Count up mode

    //-- Timeout timer --
    TA1CCR0 = timeout * CLKFREQ;
    TA1CTL |= MC_1;     //-- Count up mode
    TA1CTL |= TACLR;    //-- Clear timer count

    TA1CCTL0 |= CCIE;   //-- Enable interrupts

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

#pragma vector = TIMER1_A0_VECTOR;
__interrupt void TIMER1_A0_ISR (void) {

    //-- Clear flag
    TA1CCTL0 &= ~CCIFG;

    stopGearMotor();
}

