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
    int err = 0;
    switch(cur_direction) {
    case CLOCKWISE:
        err = disengageLeft();
        break;
    case ANTICLOCKWISE:
        err = disengageRight();
        break;
    case REST:
        err = disengageBoth();
        break;

    }

    return err;
}


/**
 * Functions to control pawl movement
 */
static int disengageRight(void) {
    int pawl_right;
    int motor_increment = 0;
    int tries = 0;
    int spi_tries = 0;

    receive_hallsensors(NULL, NULL, &pawl_right);

    //-- Error checking to see if we could receive pawl_right data
    if (pawl_right < 0) return -1;

    if (pawl_right <= RIGHT_THRES) {
        //-- Already disengaged (Why?)
        return -2;
    }

    do {
        if (++tries > MAX_TRIES) return -3;

        startGearMotor(1, MEDIUM, 100);

        // TODO: moveMainMotor(/*direction*/, motor_increment);

        while (GearMotorOn);

        do{
            receive_hallsensors(NULL, NULL, &pawl_right);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (pawl_right == -1);

        spi_tries = 0;

        motor_increment++;
    } while (pawl_right > RIGHT_THRES);


    return 0;
}

static int disengageLeft(void) {
    int pawl_left;
    int motor_increment = 0;
    int tries = 0;
    int spi_tries = 0;

    receive_hallsensors(&pawl_left, NULL, NULL);

    //-- Error checking to see if we could receive pawl_right data
    if (pawl_left < 0) return -1;

    if (pawl_left <= LEFT_THRES) {
        //-- Already disengaged (Why?)
        return -2;
    }

    do {
        if (++tries > MAX_TRIES) return -3;

        //-- Backward - Medium speed
        startGearMotor(0, MEDIUM, 100);

        // TODO: moveMainMotor(/*direction*/, motor_increment);

        while (GearMotorOn);

        do{
            receive_hallsensors(&pawl_left, NULL, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (pawl_left == -1);

        spi_tries = 0;

        motor_increment++;
    } while (pawl_left > LEFT_THRES);


    return 0;
}

static int disengageBoth(void) {
    int cam;
    int tries = 0;
    int timeout = 50;
    const int offset = 14781;
    int spi_tries = 0;

    receive_hallsensors(NULL, &cam, NULL);

    if (cam < 0) return -1;

    cam -= offset;

    if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
        //-- Already disengaged (Why?)
        return -2;
    }

    do {
        if (cam > 0) {
            //-- Move gear motor backward
            startGearMotor(0, SLOW, timeout);
        } else {
            startGearMotor(1, SLOW, timeout);
        }

        //-- Wait until gear motor stops
        while (GearMotorOn);

        do{
            receive_hallsensors(NULL, &cam, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (cam == -1);

        spi_tries = 0;

        cam -= offset;

        if (++tries > MAX_TRIES) return -3;

        timeout -= 5;
    } while (cam > CAM_THRES_UPPER || cam < CAM_THRES_LOWER);

    return 0;
}


