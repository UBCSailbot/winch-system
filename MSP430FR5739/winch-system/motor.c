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
unsigned int motor_tries = 0;

/**
 * P2.2 DIR: 1 forward and 0 backward TODO: confirm
 *
 * P3.4 STEP: Uses TB1 CCR1 capture register
 */
void init_Main_Motor(void) {

    motor_increment = 0;

    //-- Fill initial value in motor status struct
    motor_stat.power = OFF;
    motor_stat.position = 0;
    motor_stat.direction = REST;
    motor_stat.setpoint = 180;

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

    //-- THIS ONLY WORKS if init_Main_Motor is called after init_spi
    setCurrentPosition();
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

    //-- Wait until the increment is over 1000 Hz
    while (isMotorRunning()) __delay_cycles(1000);

    return 0;
}
/**
 * Rotates the main motor in the direction indicated by the
 * global state
 *
 * Returns -1 when error and 0 when success and 1 if motor has reached setpoint
 */
int setMainMotorPosition(unsigned int phase) {
    int ret;

    if (phase == INIT_MMOTOR) {

        if (motor_stat.setpoint > 360) return -1;

        //-- Motor should have already gone through the TURN_MOTOR_ON state
        if (!isMotorOn()) return -2;

        //-- Set DIR pin
        switch(motor_stat.direction) {
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

        ret = setDirectionToMove(motor_stat.setpoint);
        if (ret < 0) return -2;

        if (motor_stat.direction == REST) {

            //-- This stops it from moving in the specified direction
            stopMainMotor();

            //-- We don't want to power off the motor as it should retain its position until pawls are engaged
            //turnOffMotor();
            return 1;
        }

        // If the direction changed move back to Start Pawl
        if (ret == 1) {

            //-- This stops it from moving in the specified direction (Motor still powered)
            stopMainMotor();

            return 2;
            //if (++motor_tries > MAX_MOTOR_TRIES) return -5;
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
    motor_stat.power = ON;
}

void turnOffMotor(void) {
    P1OUT &= ~ON_MOTOR;     // Disable Motor through motor controller
    motor_stat.power = OFF;
}

int isMotorOn(void) {
    return motor_stat.power;
}

int isMotorRunning(void) {
    return TB1CTL & MC_1;
}

/**
 * Gets the position stored in motor_stat struct
 * from the last call to setCurrentPosition()
 *
 * Returns mmain motor position from last update
 */
unsigned int getCurrentCachedPosition(void) {
    return motor_stat.position;
}

/**
 * Calculated position from potentiometer value
 * and stores the postion into the motor_stat struct
 *
 * Returns 0 when successful and < 0 when error
 */
int setCurrentPosition(void) {
    unsigned int voltage;
    unsigned int position;
    int err;

    err = receive_potentiometer(&voltage);
    if (err) return err;

    position = (unsigned int) CALC_POS(voltage);

    if (position > 360) return -1;

    motor_stat.position = position;

    return 0;
}

/**
 * Gets the last updated direction value from the
 * call to setDirectionToMove()
 */
unsigned int getCurrentCachedDirectionToMove(void) {
    return motor_stat.direction;
}

/**
 * This function is used to figure out what direction the motor should move
 * It changes it the motor status to reflect the calculated direction
 *
 * return < 0 when error
 *          0 when action is successful and direction to move to did not change
 *          1 when action is successful and direction to move to changed
 */
int setDirectionToMove(unsigned int setpoint) {
    int err;
    unsigned int temp_direction;

<<<<<<< Updated upstream
    temp_direction = motor_stat.direction;
=======
    setpoint = CALC_VOLT(position);
>>>>>>> Stashed changes

    motor_stat.setpoint = setpoint;

    err = setCurrentPosition();
    if (err) return err;

    if (motor_stat.position == setpoint) {
        //-- Position Reached
        motor_stat.direction = REST;
    } else {
        motor_stat.direction = motor_stat.position < setpoint ? CLOCKWISE : ANTICLOCKWISE;
    }

    return temp_direction != motor_stat.direction;
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


