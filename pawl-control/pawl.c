/*
 * pawl.c
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#include "pawl.h"
#include <msp430.h>
#include "gearmotor.h"

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
 * Return: 0 when success and -1 otherwise
 */
int move_pawl(void) {
    switch(cur_direction) {
    case CLOCKWISE:
        disengageRight();
        break;
    case ANTICLOCKWISE:
        disengageLeft();
        break;
    case REST:
        disengageBoth();
        break;

    }
}


/**
 * Functions to control pawl movement
 */
static int disengageRight(void) {
    int pawl_left, cam, pawl_right;
    int motor_increment = 0;
    int tries = 0;

    receive_hallsensors(NULL, NULL, &pawl_right);

    //-- Error checking to see if we could receive pawl_right data
    if (pawl_right < 0) return -1;

    if (pawl_right <= RIGHT_THRES) {
        //-- Already disengaged (Why?)
        return -2;
    }

    do {
        startGearMotor(/*forward*/, /*SPEED*/, /*Timeout*/);

        moveMainMotor(/*direction*/, motor_increment);

        /* Wait until gear motor timeout */

        receive_hallsensors(NULL, NULL, &pawl_right);

        if (++tries > /*Max motor tries*/) return -3;
        motor_increment++;
    } while (pawl_right > RIGHT_THRES);


    return 0;
}

static int disengageLeft(void) {
    int pawl_left;
    int motor_increment = 0;
    int tries = 0;

    receive_hallsensors(NULL, NULL, &pawl_left);

    //-- Error checking to see if we could receive pawl_right data
    if (pawl_left < 0) return -1;

    if (pawl_left <= LEFT_THRES) {
        //-- Already disengaged (Why?)
        return -2;
    }

    do {
        //-- Backward - Medium speed
        startGearMotor(0, MEDIUM, /*Timeout*/);

        moveMainMotor(/*direction*/, motor_increment);

        while (isGearMotorOn);

        receive_hallsensors(NULL, NULL, &pawl_left);

        if (++tries > /*Max motor tries*/) return -3;
        motor_increment++;
    } while (pawl_left > Threshold);


    return 0;
}

static int disengageBoth(void) {
    // TODO: Moving Cam to center position using the negative or positive value read from SPI
}


