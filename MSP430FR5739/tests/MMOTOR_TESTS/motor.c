/*
 * motor.c
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include "motor.h"
#include "spi.h"
#include "debug.h"

int motor_increment;
unsigned int motor_step_count;
volatile int motor_state;
volatile int prev_position;
volatile int curr_position;
int direction;
volatile unsigned int fault;
int difference = 0;


// motor control variables for ramping
char accel = 0;
char motor_inc_status = 0;
/**
 * P2.2 DIR: 1 forward and 0 backward TODO: confirm
 *
 * P3.4 STEP: Uses TB1 CCR1 capture register
 */
void init_Main_Motor(void) {

    motor_increment = 0;
    motor_step_count = 0;
    motor_state = OFF;
    fault = 0;
    direction = REST;

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
    P3DIR |= ON_MOTOR;
    P3OUT &= ~ON_MOTOR;
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
    P3OUT |= ON_MOTOR;

    motor_increment = increment;

    TB1CTL |= TBCLR;                // Clear timer count
    TB1CTL |= MC_1;                 // Count up mode
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
    motor_state = ON;

    motor_inc_status = 1;

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
    unsigned int voltage;
    int err;
    int setpoint;
    int tries = 0;

    setpoint = CALC_VOLT(position);
    prev_position = getCurrentPosition();

    if (position > 360 || position < 0) return -1;

    //-- Receive Pot voltage and determine the direction
    do {
        err = receive_potentiometer(&voltage);

    } while (err < 0);


    if (voltage == setpoint) {
        //-- Position Reached
        return -3;
    } else {
        direction = voltage < setpoint ? ANTICLOCKWISE : CLOCKWISE;
    }

    //-- Set DIR pin
    switch(direction) {
    case CLOCKWISE:
        V_PRINTF("CLK");
        P2OUT |= DIR;
        break;

    case ANTICLOCKWISE:
        V_PRINTF("ACLK");
        P2OUT &= ~DIR;
        break;

    default:
        return -4;  // Action not completed
    }

    //-- Enable motor through motor controller
    P3OUT |= ON_MOTOR;

    //******* Set speed **** [ML]
    TB1CCR0 = UPPER_COUNT_SUPER_SLOW - 1;
    TB1CCR1 = UPPER_COUNT_SUPER_SLOW;
    motor_step_count = 0;


    TB1CTL |= TBCLR;                // Clear timer count
    TB1CTL |= MC_1;                 // Count up mode
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
    motor_state = ON;

    accel = 1;

    TB1CCTL1 |= CCIE;   // Enable interrupts on reg 1
    TB1CCTL0 |= CCIE;   // Enable interrupts on reg 0

    /*
     * Receive potentiometer voltage. Stop motor when setpoint reached
     * Get data at 1kHz frequency
     */
    do {

        if (checkMotorFault()) {
            V_PRINTF("MMOTOR FAULT : %d \r\n", difference);
            stopMainMotor();
            return -1;
        }
        do {
            err = receive_potentiometer(&voltage);

        } while (err < 0);

        curr_position = CALC_POS(voltage);

        if (direction == CLOCKWISE && voltage < setpoint || direction == ANTICLOCKWISE && voltage > setpoint) {
            //-- Toggle the DIR pin
            P2OUT ^= DIR;

            //-- Toggle the direction
            direction ^= CLOCKWISE ^ ANTICLOCKWISE;

            if (++tries > MAX_MOTOR_TRIES) return -5;
        }

        if (voltage == setpoint){
            accel = 0;

            while (isMotorOn()) __delay_cycles(10000);


        }

        V_PRINTF("pos: %d \r\n", curr_position);
        __delay_cycles(1000); // 1kHz freq
    }while (voltage != setpoint);

    stopMainMotor();


    return 0;
}

void stopMainMotor(void) {
    TB1CTL &= ~MC_1;        // Hault PWM timer
    TB1CTL |= TBCLR;        // Clear timer count
    P3OUT &= ~ON_MOTOR;     // Disable Motor through motor controller

    TB1CCTL1 |= OUTMOD_0;    // Toggle reset mode
    TB1CCTL1 &= ~OUT;        // Force output to zero
    motor_state = OFF;

    TB1CCTL1 &= ~CCIE;
    TB1CCTL0 &= ~CCIE;  // Disable interrupts reg 0
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

unsigned int checkMotorFault(void) {
    unsigned int fault_tmp = fault;
    fault = 0;

    return fault_tmp;
}


#pragma vector = TIMER1_B0_VECTOR;
__interrupt void TIMER1_B0_ISR (void) {
    int diff;

    if (motor_inc_status)
    {
        //-- Decrement no. of steps, if zero stop motor
        if (motor_increment <= 0) {
            stopMainMotor();
            motor_increment = 0;
            motor_inc_status = 0;
        } else {
            motor_increment--;
        }
    }
    else // Ramping
    {

        switch (direction) {
        case CLOCKWISE:
            diff = prev_position - curr_position;
            break;

        case ANTICLOCKWISE:
            diff = curr_position - prev_position;
            break;

        case REST:
        default:
            //-- VALIDATE THIS
            diff = 0;
            break;
        }

        // Speed up or speed down
        if (accel)
        {
            if (TB1CCR0 > UPPER_COUNT_MID)
            {
                //if (diff == 0) TB1CCR0 = UPPER_COUNT_SLOW;
                 TB1CCR0 = TB1CCR0 >> 1;
            }
            else
            {
                TB1CCR0 = UPPER_COUNT_MID;
            }
        }
        else
        {
            if (TB1CCR0 < UPPER_COUNT_SUPER_SLOW)
            {
                //if (diff == 0) TB1CCR0 = UPPER_COUNT_SLOW;
                 TB1CCR0 = (TB1CCR0 << 4);
            }
            else
            {
                TB1CCR0 = UPPER_COUNT_SUPER_SLOW;
                motor_step_count = 0;
                accel = 0;
                stopMainMotor();
            }
        }

        TB1CCR1 = TB1CCR0 >> 1;


    }


    TB1CCTL0 &= ~CCIFG;     // Clear interrupt flag
    //TB1CTL &= ~(TBIFG);
}

#pragma vector = TIMER1_B1_VECTOR;
__interrupt void TIMER1_B1_ISR (void) {
    int diff;

    switch(__even_in_range(TB1IV, 12)) {
    case 0x00:
        break;

    case 0x02:  // TB1CCR1
//        if (++motor_step_count % STEP_COUNT_FOR_MOTOR_CHECK == 0)
//        {
//
//            switch (direction) {
//            case CLOCKWISE:
//                diff = prev_position - curr_position;
//                break;
//
//            case ANTICLOCKWISE:
//                diff = curr_position - prev_position;
//                break;
//
//            case REST:
//            default:
//                //-- VALIDATE THIS
//                diff = 0;
//                break;
//            }
//
//            if ( diff > 6 + 1 || diff < 6 - 1 ){
//                difference = diff;
//                fault = 1;
//            }
//
//            prev_position = curr_position;
//        }

        ++motor_step_count;
        break;

    default:
        break;
    }

    TB1CCTL1 &= ~CCIFG;     // Clear interrupt flag
}


