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
int move_pawl(unsigned int phase) {
    unsigned int direction;
    int ret = 0;

    direction = getCurrentCachedDirection();

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

    }

    return ret;
}

static int disengageRight(unsigned int phase) {
    unsigned int pawl_right;
    int err;
    unsigned int spi_tries = 0;

    if (phase == INIT_PAWL) {

        //-- If the main motor is on do not continue
        if (isMotorOn()) {
            return -1;
        }

        motor_inc_tries = 0;

        //-- Turn the motor on until it reaches
        startGearMotor(FORWARD, MEDIUM, 200);

    } else {    //-- RUN_PAWL

        do{
            err = receive_hallsensors(NULL, NULL, &pawl_right);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err);

        if (pawl_right <= RIGHT_THRES) {
            stopGearMotor();
            return 1;
        }

        if (!isGearMotorOn() && !isMotorOn()) {

            if (++motor_inc_tries > MAX_TRIES) return -3;

            incrementMainMotor(CLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(FORWARD, MEDIUM, 200);
        }

    }

    return 0;
}

static int disengageLeft(unsigned int phase) {
    unsigned int pawl_left;
    unsigned int spi_tries = 0;
    int err;

    if (phase == INIT_PAWL) {

        //-- If the main motor is on do not continue
        if (isMotorOn()) {
            return -1;
        }

        motor_inc_tries = 0;

        startGearMotor(BACKWARD, MEDIUM, 200);
    } else {    //-- RUN_PAWL

        do{
            err = receive_hallsensors(&pawl_left, NULL, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err);

        if (pawl_left <= LEFT_THRES) {
            stopGearMotor();
            return 1;
        }

        if (!isGearMotorOn() && !isMotorOn()) {

            //-- If gear motor has timed out
            if (++motor_inc_tries > MAX_TRIES) return -3;

            incrementMainMotor(ANTICLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(BACKWARD, MEDIUM, 200);
        }
    }

    return 0;
}

int engageBoth(unsigned int phase) {
    int cam;
    int err;
    unsigned int spi_tries = 0;

    if (phase == INIT_PAWL) {

        //-- set direction to backward
        dir = BACKWARD;

        err = receive_hallsensors(NULL, &cam, NULL);

        if (err < 0) return -1;

        if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
            //-- Already disengaged (Why?)
            return 0;
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

    } else {    //-- RUN_PAWL

        do{
            err = receive_hallsensors(NULL, &cam, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err == -1);

        if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
            stopGearMotor();

            //-- Move in the reverse direction to counter-act enertia
            dir ^= FORWARD;
            startGearMotor(dir, SLOW, 75);
            return 1;
        }

        //-- If cam value is positive and dir is forward. Reverse
        if (cam < CAM_MID && dir == BACKWARD || cam > CAM_MID && dir == FORWARD) {
            stopGearMotor();

            //-- toggle direction
            dir ^= FORWARD;

            //-- Reverse gear motor
            startGearMotor(dir, SLOW, 1000);
        }

        if (!isGearMotorOn()) {
            if (++motor_inc_tries > MAX_TRIES) return -3;

            startGearMotor(dir, SLOW, 1000);
        }

    }

    return 0;
}


