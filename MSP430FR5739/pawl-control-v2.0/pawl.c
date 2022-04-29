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
    unsigned int pawl_right;
    int err;
    int tries = 0;
    int spi_tries = 0;

    err = receive_hallsensors(NULL, NULL, &pawl_right);



    //-- Error checking to see if we could receive pawl_right data
    if (pawl_right < 0) return -1;

    if (pawl_right <= RIGHT_THRES) {
        //-- Already disengaged (Why?)
        return 0;
    }

    //-- Start the gear motor for TODO: figure out how long
    startGearMotor(1, MEDIUM, 1000);

    do {

        if (!isGearMotorOn()) {
            //-- If gear motor has timed out
            if (++tries > MAX_TRIES) return -3;

            incrementMainMotor(CLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(1, MEDIUM, 100);
        }

        //- 10 ms
        __delay_cycles(10000);

        do{
            err = receive_hallsensors(NULL, NULL, &pawl_right);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err);

        spi_tries = 0;

        V_PRINTF("Pawl right: %d\r\n", pawl_right);

    } while (pawl_right & 0xFF > RIGHT_THRES);

    stopGearMotor();

    return 0;
}

static int disengageLeft(void) {
    unsigned int pawl_left;
    int err;
    int tries = 0;
    int spi_tries = 0;

    err = receive_hallsensors(&pawl_left, NULL, NULL);


    //-- Error checking to see if we could receive pawl_right data
    if (err < 0) return -1;

    if (pawl_left <= LEFT_THRES) {
        //-- Already disengaged (Why?)
        V_PRINTF("Pawl left: %x\r\n", pawl_left);
        return 0;
    }

    //-- Start the gear motor for TODO: figure out how long
    startGearMotor(0, MEDIUM, 1000);

    do {

        if (!isGearMotorOn()) {
            //-- If gear motor has timed out
            if (++tries > MAX_TRIES) return -3;

            incrementMainMotor(ANTICLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(0, MEDIUM, 100);
        }

        do{
            err = receive_hallsensors(&pawl_left, NULL, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err);

        spi_tries = 0;
        V_PRINTF("Pawl left: %x\r\n", pawl_left);
    } while (pawl_left > LEFT_THRES);

    stopGearMotor();

    return 0;
}

static int disengageBoth(void) {
    int cam;
    int err;
    int tries = 0;
    int timeout = 50;
    const int offset = 400;
    int spi_tries = 0;
    unsigned int dir = 0;

    err = receive_hallsensors(NULL, &cam, NULL);

    if (err < 0) return -1;

    cam -= offset;
    V_PRINTF("Cam: %d\r\n", cam);
    if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
        //-- Already disengaged (Why?)
        return -2;
    }

    if (cam > 0) {
        //-- Go backward
        dir = 0;
    } else {
        //-- Go forward
        dir = 1;
    }

    startGearMotor(dir, SLOW, timeout);

    do {

        if (!isGearMotorOn()) {
            if (++tries > MAX_TRIES) return -3;

            startGearMotor(dir, SLOW, timeout);
        }

        //-- If cam value is positive and dir is forward. Reverse
        if (cam > 0 && dir || cam < 0 && !dir) {
            stopGearMotor();
            dir ^= 1;
            //-- Reverse gear motor
            startGearMotor(dir, SLOW, timeout);
        }

        //- 10 ms
        __delay_cycles(10000);

        do{
            err = receive_hallsensors(NULL, &cam, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err);

        spi_tries = 0;

        cam -= offset;

        //timeout -= 5;
    } while (cam > CAM_THRES_UPPER || cam < CAM_THRES_LOWER);

    stopGearMotor();

    return 0;
}


