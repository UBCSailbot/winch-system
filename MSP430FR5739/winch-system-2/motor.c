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
unsigned int direction;
unsigned int motor_tries = 0;
unsigned int setpoint;


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

int incrementMainMotor(int dir, int increment) {

    //-- Motor should have already gone through the TURN_MOTOR_ON state
    if (!isMotorOn()) return -1;

    //-- Set DIR pin
    switch(dir) {
    case CLOCKWISE:
        P2OUT &= ~DIR;
        break;

    case ANTICLOCKWISE:
        P2OUT |= DIR;
        break;

    default:
        return -2;  // Action not completed
    }

    motor_increment = increment;

    startMainMotor();

    TB1CCTL0 |= CCIE;   // Enable interrupts on reg 0

    return 0;
}
/**
 * Rotates the main motor in the direction indicated by the
 * global state
 *
 * Returns -1 when error and 0 when success and 1 if motor has reached setpoint
 */
int setMainMotorPosition(unsigned int position, unsigned int * dir, unsigned int phase) {

    unsigned int voltage;
    int err;

    if (phase == INIT_MMOTOR) {
        setpoint = (position * POT_SCALAR) + 500;

        if (position > 360) return -1;

        //-- Motor should have already gone through the TURN_MOTOR_ON state
        if (!isMotorOn()) return -2;

        direction = *dir;

        //-- Set DIR pin
        switch(direction) {
        case CLOCKWISE:
            P2OUT &= ~DIR;
            break;

        case ANTICLOCKWISE:
            P2OUT |= DIR;
            break;

        case REST:
            //-- Already at position
            return 1;

        default:
            return -4;  // Action not completed
        }

        //-- Init the tries to 0
        motor_tries = 0;

        startMainMotor();

    } else {    // PHASE == RUN_MMOTOR

        err = receive_potentiometer(&voltage);
        if (err < 0) return -2;

        if (direction == CLOCKWISE && voltage > setpoint || direction == ANTICLOCKWISE && voltage < setpoint) {
            //-- Stops the motor before switching its direction
            stopMainMotor();

            direction ^= ANTICLOCKWISE ^ CLOCKWISE;

            //-- Toggle direction
            *dir = direction;

            return 2;
            //if (++motor_tries > MAX_MOTOR_TRIES) return -5;
        }

        if (voltage <= setpoint + 25 && voltage >= setpoint - 25) {
            stopMainMotor();
            return 1;
        }
    }

    return 0;
}

void stopMainMotor(void) {
    TB1CTL &= ~MC_1;        // Hault PWM timer
    TB1CTL |= TBCLR;        // Clear timer count

    TB1CCTL1 |= OUTMOD_0;    // Toggle reset mode       [TBD: Look into these modes]
    TB1CCTL1 &= ~OUT;        // Force output to zero
}

static void startMainMotor(void) {
    //-- Starts the PWM timer
    TB1CTL |= TBCLR;                // Clear timer count
    TB1CTL |= MC_1;                 // Count up mode
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
}

void turnOnMotor(void) {
    P1OUT |= ON_MOTOR;
    motor_state = ON;
}

void turnOffMotor(void) {
    P1OUT &= ~ON_MOTOR;     // Disable Motor through motor controller
    motor_state = OFF;
}

int isMotorOn(void) {
    return motor_state;
}

int isMotorRunning(void) {
    return TB1CTL & MC_1;
}

int getCurrentPosition(unsigned int * position) {
    unsigned int voltage;
    int err;

    err = receive_potentiometer(&voltage);
    if (err) return err;

    *position = (unsigned int) ( (voltage - 500)/POT_SCALAR );

    return 0;
}

unsigned int getDirection(unsigned int position, unsigned int * dir) {
    unsigned int voltage;
    int err;
    unsigned int setpoint;

    setpoint = (position * POT_SCALAR) + 500;

    err = receive_potentiometer(&voltage);
    if (err) return err;

    if (voltage == setpoint) {
        //-- Position Reached
        *dir = REST;
    } else {
        *dir = voltage < setpoint ? CLOCKWISE : ANTICLOCKWISE;
    }

    return 0;
}


#pragma vector = TIMER1_B0_VECTOR;
__interrupt void TIMER1_B0_ISR (void) {

    //-- Decrement no. of steps, if zero stop motor
    if (motor_increment <= 0) {
        TB1CCTL0 &= ~CCIE;  // Disable interrupts
        stopMainMotor();

        //-- Do not turn power off to the motor because it needs to retain its position
        //turnOffMotor();

        motor_increment = 0;
    } else {
        motor_increment--;
    }

    TB1CCTL0 &= ~CCIFG;     // Clear interrupt flag
    TB1CTL &= ~(TBIFG);
}


