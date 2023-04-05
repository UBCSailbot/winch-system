/*
 * motor.c
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include "motor.h"
#include "debug.h"
#include "error.h"

int motor_increment;
unsigned int motor_tries = 0;
int difference;
unsigned char WAIT_RAMPING_DOWN = 0;


/**
 *  Name:       init_Main_Motor
 *
 *
 *  Purpose:    initialize the STEP pin to perform PWM using TB1.1 timer register
 *
 *  Params:     none
 *
 *  Return:     none
 *
 *  Notes:      must be called after init_spi()
 *
 *              P2.2 DIR: 1 CLK and 0 ACLK
 *              P3.4 STEP: Uses TB1 CCR1 capture register
 */
void init_Main_Motor(void) {

    motor_increment = 0;

    //-- Fill initial value in motor status struct
    motor_stat.power = OFF;
    motor_stat.position = 0;
    motor_stat.direction = REST;
    motor_stat.setpoint = 180;

    motor_tracker.fault = 0;
    motor_tracker.last_position = 0;
    motor_tracker.steps = 0;

    //-- Init DIR port to output
    P2DIR |= DIR;
    P2OUT &= ~DIR;

    //-- Initialize PWM on STEP port
    //-- P3.4 STEP  function - TB1.1 (Table 6-47 in datasheet)
    P3DIR |= STEP;
    P3SEL1 &= ~STEP;
    P3SEL0 |= STEP;
    P3OUT &= ~STEP;

    setMotorSpeed(MMOTOR_SUPER_SLOW);

    //-- TB1 reg 1 timer setup
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode
    TB1CTL |= TBSSEL_2;             // SMCLK 1 Mhz

    //-- Enable port that is connected to input 4 on the motor controller
    P1DIR |= ON_MOTOR;
    P1REN |= ON_MOTOR;
    P1OUT &= ~ON_MOTOR;

    setCurrentPosition();
}

/**
 *  Name:       incrementMainMotor
 *
 *
 *  Purpose:    increments the motor by a certain number of steps
 *
 *  Params:     dir - CLOCKWISE
 *                    ANTICLOCKWISE
 *
*               increment - number of steps (PWM pulses)
 *
 *  Return:     0 - success
 *              < 0 - failure
 *
 *  Notes:      none
 */
int incrementMainMotor(int dir, int increment) {

    //-- Motor should have already gone through the TURN_MOTOR_ON state
    if (!isMotorOn()) {
        set_error(MOTOR_NOT_ON);
        return -1;
    }

    //-- Set DIR pin
    switch(dir) {
    case CLOCKWISE:
        P2OUT &= ~DIR;
        break;

    case ANTICLOCKWISE:
        P2OUT |= DIR;
        break;

    default:
        set_error(INVALID_DIR);
        return -2;  // Action not completed
    }

    motor_increment = increment;

    setMotorSpeed(MMOTOR_SUPER_SLOW);

    motor_stat.motor_inc_stat = 1;

    startMainMotor();

    TB1CCTL0 |= CCIE;   // Enable interrupts on reg 0

    //-- Wait until the increment is over 1000 Hz
    while (isMotorRunning()) __delay_cycles(1000);

    return 0;
}

/**
 *  Name:       setMainMotorPosition
 *
 *
 *  Purpose:    sets the position of the main motor
 *
 *  Params:     phase - INIT_MMOTOR
 *                    - RUN_MMOTOR
 *
 *  Return:     COMPLETE - success
 *              ERROR - error
 *              RESTART - go to Start Pawl state
 *              RUN_AGAIN - run current phase again
 *
 *  Notes:      motor_stat struct controls what position the motor is moving to
 */
t_ret_code setMainMotorPosition(unsigned int phase) {
    int ret;

    if (phase == INIT_MMOTOR) {
        WAIT_RAMPING_DOWN = 0;
        motor_tracker.fault = 0;

        if (motor_stat.setpoint > 360){
            set_error(INVALID_SETPOINT);
            return ERROR;
        }

        //-- Motor should have already gone through the TURN_MOTOR_ON state
        if (!isMotorOn()) {
            set_error(MOTOR_NOT_ON);
            return ERROR;
        }

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
            return COMPLETE;

        default:
            set_error(INVALID_DIR);
            return ERROR;  // Action not completed
        }

        //-- Init the tries to 0
        motor_tries = 0;

        setMotorSpeed(MMOTOR_SLOW);

        motor_stat.accel = 1;
        motor_stat.motor_inc_stat = 0;

        startMainMotor();

        TB1CCTL0 |= CCIE;   // Enable interrupts on reg 0
        TB1CCTL1 |= CCIE;   // Enable interrupts on reg 1

        return COMPLETE;

    } else {    // PHASE == RUN_MMOTOR

        //-- Motor should have already gone through the TURN_MOTOR_ON state
        if (!isMotorOn()) {
            set_error(MOTOR_NOT_ON);
            return ERROR;
        }

        if (checkMotorFaultAndClear()) {
            V_PRINTF("FAULT diff:%d dir:%d", difference, motor_stat.direction)
            stopMainMotor();
            set_error(MMOTOR_FAULT);
            return ERROR;
        }

        ret = setDirectionToMove(motor_stat.setpoint);

        if (WAIT_RAMPING_DOWN) goto RAMP_WAIT;

        if (ret < 0) {
            set_error(SET_DIR_TO_MOVE_ERROR);
            return ERROR;
        }

        if (motor_stat.direction == REST) {

            //-- This stops it from moving in the specified direction
            motor_stat.accel = 0;

            WAIT_RAMPING_DOWN = 1;
RAMP_WAIT:
            if (motor_stat.running) return RUN_AGAIN;

            //-- We don't want to power off the motor as it should retain its position until pawls are engaged
            //turnOffMotor();

            WAIT_RAMPING_DOWN = 0;
            return COMPLETE;
        }

        // If the direction changed move back to Start Pawl
        if (ret == 1) {

            //-- This stops it from moving in the specified direction (Motor still powered)
            stopMainMotor();

            return RESTART;
            //if (++motor_tries > MAX_MOTOR_TRIES) return -5;
        }

        return RUN_AGAIN;
    }
}

void stopMainMotor(void) {
    TB1CTL &= ~MC_1;        // Hault PWM timer
    TB1CTL |= TBCLR;        // Clear timer count

    TB1CCTL1 &= ~OUTMOD_2;    // Output mode
    TB1CCTL1 &= ~OUT;        // Force output to zero

    TB1CCTL1 &= ~CCIE;
    TB1CCTL0 &= ~CCIE;

    motor_stat.running = 0;
    motor_stat.accel = 0;
}

static void startMainMotor(void) {
    //-- Starts the PWM timer
    TB1CTL |= TBCLR;                // Clear timer count
    TB1CTL |= MC_1;                 // Count up mode
    TB1CCTL1 |= OUTMOD_2;           // Toggle reset mode

    motor_stat.running = 1;

    //-- Update the last known position before safety check occurs
    motor_tracker.last_position = motor_stat.position;

    motor_tracker.steps = 0;
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
    if (err) {
        set_error(RECEIVE_POT_ERROR);
        return err;
    }

    position = (unsigned int) CALC_POS(voltage);

    if (position > 360) {
        set_error(CURR_POSITION_EXCEED_360);
        return -1;
    }

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
    temp_direction = motor_stat.direction;

    motor_stat.setpoint = setpoint;

    err = setCurrentPosition();
    if (err) {
        set_error(SET_CURRENT_POS_ERROR);
        return err;
    }

    if (motor_stat.position == setpoint) {
        //-- Position Reached
        motor_stat.direction = REST;
    } else {
        motor_stat.direction = motor_stat.position < setpoint ? CLOCKWISE : ANTICLOCKWISE;
    }

    return temp_direction != motor_stat.direction;
}

void setMotorSpeed(motor_speed_t speed_sel) {
    switch(speed_sel) {
    case MMOTOR_FAST:
        //-- TB1 reg 1 timer setup
        TB1CCR0 = UPPER_COUNT_FAST - 1;
        TB1CCR1 = MID_COUNT_FAST;
        break;

    case MMOTOR_MID:
        //-- TB1 reg 1 timer setup
        TB1CCR0 = UPPER_COUNT_MID - 1;
        TB1CCR1 = MID_COUNT_MID;
        break;

    case MMOTOR_SLOW:
        //-- TB1 reg 1 timer setup
        TB1CCR0 = UPPER_COUNT_SLOW - 1;
        TB1CCR1 = MID_COUNT_SLOW;
        break;

    case MMOTOR_SUPER_SLOW:
    default:
        //-- TB1 reg 1 timer setup
        TB1CCR0 = UPPER_COUNT_SUPER_SLOW - 1;
        TB1CCR1 = MID_COUNT_SUPER_SLOW;
        break;
    }
}

unsigned char checkMotorFaultAndClear(void) {
    unsigned char fault_tmp = motor_tracker.fault;
    motor_tracker.fault = 0;

    return fault_tmp;
}


#pragma vector = TIMER1_B0_VECTOR;
__interrupt void TIMER1_B0_ISR (void) {

    if (motor_stat.motor_inc_stat)
    {
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
    }
    else
    {
        if (motor_stat.accel)
        {
            if (TB1CCR0 > UPPER_COUNT_MIN)
            {
                //if (diff == 0) TB1CCR0 = UPPER_COUNT_SLOW;
                TB1CCR0 = TB1CCR0 - UPPER_COUNT_DEC;
                if (TB1CCR0 == 0) TB1CCR0 = UPPER_COUNT_MIN;
            }
            else
            {
                TB1CCR0 = UPPER_COUNT_MIN;
            }
        }
        else
        {
            if (TB1CCR0 < UPPER_COUNT_MAX)
            {
                //if (diff == 0) TB1CCR0 = UPPER_COUNT_SLOW;
                 TB1CCR0 = TB1CCR0 + UPPER_COUNT_INC;
                 if (TB1CCR0 == 0) TB1CCR0 = UPPER_COUNT_MAX;
            }
            else
            {
                TB1CCR0 = UPPER_COUNT_MAX;
                stopMainMotor();
            }
        }

        TB1CCR1 = TB1CCR0 >> 2;
    }

    TB1CCTL0 &= ~CCIFG;     // Clear interrupt flag
    TB1CTL &= ~(TBIFG);
}

#pragma vector = TIMER1_B1_VECTOR;
__interrupt void TIMER1_B1_ISR (void) {
    int diff = 0;

    switch(__even_in_range(TB1IV, 12)) {
    case 0x00:
        break;

    case 0x02:              //-- TB1CCR1
        if (++motor_tracker.steps == STEP_COUNT_FOR_MOTOR_CHECK)
        {
            motor_tracker.steps = 0;

            switch (motor_stat.direction) {
            case CLOCKWISE:
                diff = motor_stat.position - motor_tracker.last_position;
                break;

            case ANTICLOCKWISE:
                diff = motor_tracker.last_position - motor_stat.position;
                break;

            case REST:
            default:
                //-- VALIDATE THIS FORCE IT TO IGNORE CHECK
                diff = EXPECTED_POS_DIFF;
                break;
            }

            if ( diff > EXPECTED_POS_DIFF + POS_FAULT_LIMIT || diff < EXPECTED_POS_DIFF - POS_FAULT_LIMIT ) {
                motor_tracker.fault = 1;
                TB1CCTL1 &= ~CCIE;
            }

            motor_tracker.last_position = motor_stat.position;
            difference = diff;
        }
        break;

    default:
        break;
    }


    TB1CCTL1 &= ~CCIFG;     // Clear interrupt flag
}

