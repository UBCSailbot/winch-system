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
    case ROTATE_CW:
        err = test_func();
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
    if (err < 0) return -1;

    if (pawl_right <= RIGHT_THRES) {
        //-- Already disengaged (Why?)
        V_PRINTF("Pawl right: %x\r\n", pawl_right);
        return 0;
    }

    startGearMotor(1, MEDIUM, 1000);

    do {

        if (!isGearMotorOn()) {
            //-- If gear motor has timed out
            if (++tries > MAX_TRIES) return -3;

            incrementMainMotor(CLOCKWISE, 5);

            //-- Start gear motor again
            startGearMotor(1, MEDIUM, 100);
        }

        do{
            err = receive_hallsensors(NULL, NULL, &pawl_right);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err);

        spi_tries = 0;

        V_PRINTF("Pawl right: %x\r\n", pawl_right);

    } while (pawl_right > RIGHT_THRES);

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
    unsigned int spi_tries = 0;
    int motor_inc_tries = 0;
    unsigned int dir = 0;
    unsigned int speed = SLOW;
    int inc = 0;
    int speeds[3] = {SLOW, SUPER_SLOW, even_more_slow};

    //-- set direction to backward
    dir = BACKWARD;

    err = receive_hallsensors(NULL, &cam, NULL);

    if (err < 0) return -1;

    if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
        //-- Already disengaged (Why?)
        V_PRINTF("CAM: %d\r\n", cam);
        return 0;
    }

    if (cam > CAM_MID) {
        //-- Go backward
        dir = BACKWARD;
    } else {
        //-- Go forward
        dir = FORWARD;
    }

    startGearMotor(dir, speed, 1000);

    do {

        if (!isGearMotorOn()) {
            if (++motor_inc_tries > MAX_TRIES) return -3;

            speed = SLOW;

            startGearMotor(dir, SLOW, 1000);
        }

        //-- If cam value is positive and dir is forward. Reverse
        if (cam > CAM_MID && dir == FORWARD || cam < CAM_MID && dir == BACKWARD) {
            stopGearMotor();

            //-- toggle direction
            dir ^= FORWARD;

            inc = (++inc) % 3;

            //-- Reverse gear motor
            startGearMotor(dir, speeds[inc], 1000);
        }

        do{
            err = receive_hallsensors(NULL, &cam, NULL);
            if (++spi_tries > MAX_TRIES) return -4;
        } while (err == -1);

        spi_tries = 0;

        __delay_cycles(10000);

    } while (cam > CAM_THRES_UPPER || cam < CAM_THRES_LOWER);
    V_PRINTF("CAM: %d\r\n", cam);
    stopGearMotor();
    dir ^= FORWARD;
    startGearMotor(dir, SLOW, 75);

    int i = 0;
    for (i = 0; i < 5; i++) {
        err = receive_hallsensors(NULL, &cam, NULL);
        V_PRINTF("CAM: %d\r\n", cam);
        //-- 10ms
        __delay_cycles(10000);
    }

    return 0;
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

