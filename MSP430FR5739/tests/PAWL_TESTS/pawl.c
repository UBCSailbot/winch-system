/*
 * pawl.c
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#include <msp430.h>
#include <stdio.h>
#include "gearmotor.h"
#include "pawl.h"
#include "spi.h"
#include "motor.h"
#include "debug.h"

unsigned int motor_inc_tries = 0;
unsigned int dir = 0;

/**
 * Set the state of the paws to either engaged or disengaged
 * This function depends on a global state - Clockwise, AntiClockwise or REST
 * These state control the main motor is also used to determine which paw to
 * control.
 *
 * Clockwise - Drive gear motor backwards (to the right) to disengage right pawl and engage left.
 *
 * AntiClockwise - Drive gear motor forward (to the left) to disengage left pawl and engage right.
 *
 * REST - Drive gear motor to the middle position (This position is determined by the hall sensor data from left and right pawl).
 *        Depending on which Paw is engaged we move to the opposite direction (to disengage the pawl).
 *        The center is determined by CAM hall sensor data
 *
 * Return: negative when error, 0 when success and 1 when pawl position reached
 */
t_ret_code move_pawl(unsigned int phase, unsigned int direction) {
    t_ret_code ret = COMPLETE;

    switch(direction) {
    case CLOCKWISE:
        ret = disengageLeft(phase);
        break;
    case ANTICLOCKWISE:
        ret = disengageRight(phase);
        break;
    case REST:
        ret = engageBoth(phase);
        break;

    case ROTATE_CW:
        test_func();
        break;

    }

    return ret;
}

static t_ret_code disengageRight(unsigned int phase) {
    unsigned int pawl_right;
    int err;
    unsigned int spi_tries = 0;

    if (phase == INIT_PAWL) {
        err = receive_hallsensors(NULL, NULL, &pawl_right);

        //-- Error checking to see if we could receive pawl_right data
        if (err < 0) return ERROR;

        if (pawl_right <= RIGHT_THRES) {
            return COMPLETE;
        }

        //-- Keeps track of the number of times the main motor rotated to unlock the pawls
        motor_inc_tries = 0;

        //-- Turn the motor on until it reaches
        startGearMotor(FORWARD, MEDIUM, 200);

        return COMPLETE;

    } else {    //-- RUN_PAWL

        if (!isGearMotorOn()) {
            //-- If gear motor has timed out
            if (++motor_inc_tries > MAX_TRIES) return ERROR;

            incrementMainMotor(CLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(FORWARD, MEDIUM, 200);
        }


        do{
            err = receive_hallsensors(NULL, NULL, &pawl_right);
            if (++spi_tries > MAX_TRIES) return ERROR;
        } while (err);

        if (pawl_right <= RIGHT_THRES) {
            stopGearMotor();
            return COMPLETE;
        }

        return RUN_AGAIN;
    }
}

static t_ret_code disengageLeft(unsigned int phase) {
    unsigned int pawl_left;
    unsigned int spi_tries = 0;
    int err;

    if (phase == INIT_PAWL) {

        err = receive_hallsensors(&pawl_left, NULL, NULL);

        //-- Error checking to see if we could receive pawl_right data
        if (err < 0) return ERROR;

        if (pawl_left <= LEFT_THRES) {
            //-- Already disengaged (Why?)
            return COMPLETE;
        }

        motor_inc_tries = 0;

        startGearMotor(BACKWARD, MEDIUM, 200);

        return COMPLETE;
    } else {    //-- RUN_PAWL

        if (!isGearMotorOn()) {

            //-- If gear motor has timed out
            if (++motor_inc_tries > MAX_TRIES) return ERROR;

            incrementMainMotor(ANTICLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(BACKWARD, MEDIUM, 200);
        }

        do{
            err = receive_hallsensors(&pawl_left, NULL, NULL);
            if (++spi_tries > MAX_TRIES) return ERROR;
        } while (err);

        if (pawl_left <= LEFT_THRES) {
            stopGearMotor();
            return COMPLETE;
        }

        return RUN_AGAIN;
    }
}

t_ret_code engageBoth(unsigned int phase) {
    int cam;
    int err;
    unsigned int spi_tries = 0;

    if (phase == INIT_PAWL) {

        //-- set direction to backward
        dir = BACKWARD;

        err = receive_hallsensors(NULL, &cam, NULL);

        if (err < 0) return ERROR;

        if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
            //-- Already disengaged (Why?)
            return COMPLETE;
        }

        if (cam > CAM_MID) {
            //-- Go backward
            dir = BACKWARD;
        } else {
            //-- Go forward
            dir = FORWARD;
        }

        motor_inc_tries = 0;

        startGearMotor(dir, SLOW, 1000);

        return COMPLETE;

    } else {    //-- RUN_PAWL

        do{
            err = receive_hallsensors(NULL, &cam, NULL);
            if (++spi_tries > MAX_TRIES) {
                V_PRINTF("MAX_SPI: %d\r\n");
                return ERROR;
            }
        } while (err == -1);

        if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
            stopGearMotor();

            //-- Move in the reverse direction to counter-act inertia
            dir ^= FORWARD;

            startGearMotor(dir, SLOW, 75);
            return COMPLETE;
        }

        //-- If cam value is positive and dir is forward. Reverse
        if (cam < CAM_MID && dir == BACKWARD || cam > CAM_MID && dir == FORWARD) {
            stopGearMotor();

            //-- toggle direction
            dir ^= FORWARD;
            V_PRINTF("GM_DIR_CH: %d\r\n", dir);
            //-- Reverse gear motor
            startGearMotor(dir, SLOW, 1000);
        }


        if (!isGearMotorOn()) {
            if (++motor_inc_tries > MAX_TRIES) return ERROR;

            startGearMotor(dir, SLOW, 1000);

            V_PRINTF("GM_DIR: %d\r\n", dir);
        }

        return RUN_AGAIN;
    }
}

/*
 * @brief derain12: Specific function to continuously run cam and read hall sensors data
 *                  Runs for 10 seconds, reads every 1 second
 *
 */
static int test_func(void) {
    unsigned int pawl_left,pawl_right;
    signed int cam;
    int err;
    int tries = 0;
    int spi_tries = 0;

    startGearMotor(1, MEDIUM, 100);
    __delay_cycles(12500);
    startGearMotor(1, even_more_slow, 5000);

    while(isGearMotorOn()){
        err = receive_hallsensors(&pawl_left,&cam, &pawl_right);

        V_PRINTF("\r\nPAWL_LEFT: %x | CAM: %d | PAWL_RIGHT: %x\r\n", pawl_left, cam, pawl_right);
        //-- Error checking to see if we could receive pawl_right data
        if (err < 0){
            stopGearMotor();
            return -1;
        }
        //-- DCOCLK is 8MHz by default, MCLK source is DCOCLK with 1/8 divider
    }

    return 0;
}

