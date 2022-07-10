/*
 * motor.c
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include "motor.h"
#include "spi.h"

int motor_increment;
volatile int motor_state;


/**
 * P2.2 DIR: 1 forward and 0 backward TODO: confirm
 *
 * P3.4 STEP: Uses TB1 CCR1 capture register
 */
void init_Main_Motor(void) {

    motor_increment = 0;
    motor_state = OFF;

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
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
    TB1CTL |= TBSSEL_2;             // SMCLK 1 Mhz

    //-- Enable port that is connected to input 4 on the motor controller
    P1DIR |= ON_MOTOR;
    P1OUT &= ~ON_MOTOR;
}

int incrementMainMotor(int direction, int increment) {
    //-- Set DIR pin
    switch(direction) {
    case CLOCKWISE:
        P2OUT |= DIR;
        break;

    case ANTICLOCKWISE:
        P2OUT &= ~DIR;
        break;

    default:
        return -1;  // Action not completed
    }

    //-- Enable motor through motor controller
    P1OUT |= ON_MOTOR;

    motor_increment = increment;

    TB1CTL |= TBCLR;                // Clear timer count
    TB1CTL |= MC_1;                 // Count up mode
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
    motor_state = ON;

    TB1CCTL0 |= CCIE;   // Enable interrupts on reg 0

    return 0;
}
/**
 * Rotates the main motor in the direction indicated by the
 * global state
 *
 * Returns -1 when error and 0 otherwise
 */
int setMainMotorPosition(int position) {
    int direction;
    unsigned int voltage;
    int err;
    int setpoint;
    int tries = 0;

    setpoint = CALC_VOLT(position);

    if (position > 360 || position < 0) return -1;

    //-- Receive Pot voltage and determine the direction
    err = receive_potentiometer(&voltage);
    if (err < 0) return -2;

    if (voltage == setpoint) {
        //-- Position Reached
        return -3;
    } else {
        direction = voltage < setpoint ? ANTICLOCKWISE : CLOCKWISE;
    }

    //-- Set DIR pin
    switch(direction) {
    case CLOCKWISE:
        P2OUT |= DIR;
        break;

    case ANTICLOCKWISE:
        P2OUT &= ~DIR;
        break;

    default:
        return -4;  // Action not completed
    }

    //-- Enable motor through motor controller
    P1OUT |= ON_MOTOR;

    TB1CTL |= TBCLR;                // Clear timer count
    TB1CTL |= MC_1;                 // Count up mode
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
    motor_state = ON;

    /*
     * Receive potentiometer voltage. Stop motor when setpoint reached
     * Get data at 1kHz frequency
     */
    do {
        err = receive_potentiometer(&voltage);
        if (err < 0) return -2;

        if (direction == CLOCKWISE && voltage < setpoint || direction == ANTICLOCKWISE && voltage > setpoint) {
            //-- Toggle the DIR pin
            P2OUT ^= DIR;

            //-- Toggle the direction
            direction ^= CLOCKWISE ^ ANTICLOCKWISE;

            if (++tries > MAX_MOTOR_TRIES) return -5;
        }

        if (voltage == setpoint) stopMainMotor();

        __delay_cycles(1000); // 1kHz freq
    }while (voltage != setpoint);

    stopMainMotor();

    return 0;
}

void stopMainMotor(void) {
    TB1CTL &= ~MC_1;        // Hault PWM timer
    TB1CTL |= TBCLR;        // Clear timer count
    P1OUT &= ~ON_MOTOR;     // Disable Motor through motor controller

    TB1CCTL1 |= OUTMOD_0;    // Toggle reset mode
    TB1CCTL1 &= ~OUT;        // Force output to zero
    motor_state = OFF;
}

int isMotorOn(void) {
    return motor_state;
}

unsigned int getCurrentPosition(void) {
    unsigned int voltage;
    int err;

    err = receive_potentiometer(&voltage);
    if (err) return err;

    return (unsigned int) CALC_POS(voltage);
}


#pragma vector = TIMER1_B0_VECTOR;
__interrupt void TIMER1_B0_ISR (void) {

    //-- Decrement no. of steps, if zero stop motor
    if (motor_increment <= 0) {
        TB1CCTL0 &= ~CCIE;  // Disable interrupts
        stopMainMotor();
        motor_increment = 0;
    } else {
        motor_increment--;
    }

    TB1CCTL0 &= ~CCIFG;     // Clear interrupt flag
    TB1CTL &= ~(TBIFG);
}


