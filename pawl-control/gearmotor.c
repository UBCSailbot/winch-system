/*
 * gearmotor.c
 *
 *  Created on: Apr. 4, 2022
 *      Author: mlokh
 */
#include "gearmotor.h"

int isGearMotorOn = 0;


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
    TB0CCR0 = /*Upper count*/;

    //-- OUTMOD - Toggle/reset
    TB0CCTL1 |= OUTMOD_2;

    //-- Clock ACLK (32 khz)
    TB0CTL |= TBSSEL_1;


    //-- Initialize timeout timer
    //-- Timer_A Main timer 1s
    TA1CCR0 = /*Default timeout*/;

    TA1CCTL0 |= CCIE;

    //-- ACLK (32 khz) / 2
    TA1CTL |= TASSEL_1;

}

void startGearMotor(int forward, int speed, int timeout) {

    //-- Gear motor PWM --

    //-- Slow decay mode
    PJOUT |= MODE;

    //-- Direction
    if (forward) PJOUT |= PHASE;
    else PJOUT &= ~PHASE;

    //-- IC active
    PJOUT |= NSLEEP;


    //-- Timeout timer --
    //-- MC - UP
    TB0CTL |= MC_1;

    //-- Timeout timer
    TA1CCR0 = timeout;

    //-- Count up mode
    TA1CTL |= MC_1;

    //-- Set motor state to running
    isGearMotorOn = 1;
}

void stopGearMotor(void) {
    //-- MC - idle, Disable PWM
    TB0CTL &= ~MC_1;

    //-- Put into sleep
    PJOUT &= ~NSLEEP;

    //-- Disable count up mode (No timeout)
    TA1CTL &= ~MC_1;

    isGearMotorOn = 0;
}

#pragma vector = TIMER1_A0_VECTOR;
__interrupt void TIMER1_A0_ISR (void) {

    //-- Clear flag
    TA1CTL &= ~TAIFG;

    stopGearMotor();
}

