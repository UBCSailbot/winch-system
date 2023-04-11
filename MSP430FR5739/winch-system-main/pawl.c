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
#include "error.h"

unsigned int motor_inc_tries = 0;
unsigned int dir = 0;
char move_cam = 0;
unsigned int direction;

//static const t_pawl_state next_state_lookup_table[MAX_STATE][MAX_RET_CODE] =
//{
// // RUN_AGAIN       COMPLETE            ERROR           RESTART
//
// //---- GET_SPI ----
// {GET_SPI,          UPDATE_DIR,         SEND_ERROR,     GET_SPI},
//
// //---- UPDATE_DIR ----
// {UPDATE_DIR,       CHECK_EXIT_COND,    SEND_ERROR,     GET_SPI},
//
// //---- CHECK_EXIT_COND ----
// {RUN_GMOTOR,       END_ACTION,         SEND_ERROR,     GET_SPI},
//
//};

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
t_ret_code move_pawl(unsigned int phase) {
    t_ret_code ret = COMPLETE;

    switch (phase)
    {
    case INIT_PAWL:
        ret = setup_pawl();
        break;
    case RUN_PAWL:
        ret = pawl_action();
        break;
    default:
        ret = ERROR;
        break;
    }

    return ret;
}

static t_ret_code setup_pawl(void)
{
    t_ret_code ret = COMPLETE;

    if ((PJIN & NFAULT) || (PJOUT & NSLEEP)) {
        ret = COMPLETE;
    } else {
        ret = ERROR;
    }

    direction = getCurrentCachedDirectionToMove();

    switch(direction) {
    case CLOCKWISE:
        pawl_track.CAM_DIR = BACKWARD;
        pawl_track.engaged_pawl = LEFT;
        break;
    case ANTICLOCKWISE:
        pawl_track.CAM_DIR = FORWARD;
        pawl_track.engaged_pawl = RIGHT;
        break;
    case REST:
        pawl_track.CAM_DIR = FORWARD;
        pawl_track.engaged_pawl = NONE;
    default:
        set_error(INVALID_DIR);
        ret = ERROR;
        break;
    }

    return ret;
}

static t_ret_code pawl_action(void)
{
    t_ret_code ret = RUN_AGAIN;
    unsigned int pawl_val;

    //-- Perform GET SPI
    //-- Update Direction changed
    //-- Check Exit condition
    //-- Turn on Motor

    ret = get_spi(&pawl_val);
    if (ret != COMPLETE) return ret;

    ret = update_dir(pawl_val);
    if (ret != COMPLETE) return ret;

    ret = check_exit(pawl_val);
    if (ret != RUN_AGAIN) return ret;

    ret = turn_on_gmotor();
    if (ret != COMPLETE) return ret;

    return RUN_AGAIN;

}


//-- Get spi functions
static t_ret_code get_spi(unsigned int *pawl_val)
{
    t_ret_code ret = COMPLETE;
    int err;

    switch(pawl_track.engaged_pawl)
    {
    case RIGHT:
        err = receive_hallsensors(NULL, NULL, pawl_val);
        break;
    case LEFT:
        err = receive_hallsensors(pawl_val, NULL, NULL);
        break;
    case NONE:
        err = receive_hallsensors(NULL, (int *) pawl_val, NULL);
        break;
    default:
        ret = ERROR;
        return ret;
    }

    if (err < 0)
    {
        pawl_track.tries.read_spi++;
        ret = RUN_AGAIN;
    }
    else
    {
        pawl_track.tries.read_spi = 0;
        ret = COMPLETE;
    }

    if (pawl_track.tries.read_spi > MAX_TRIES) {
        set_error(MAX_SPI_TRIES);
        ret = ERROR;
    }

    return ret;
}


//-- Update dir functions
static t_ret_code update_dir(unsigned int pawl_val)
{
    t_ret_code ret = COMPLETE;

    switch(pawl_track.engaged_pawl)
    {
    case RIGHT:
    case LEFT:
        break;

    case NONE:
        if ((int)pawl_val > CAM_MID) {
            pawl_track.CAM_DIR = BACKWARD;
        } else {
            pawl_track.CAM_DIR = FORWARD;
        }
        break;
    default:
        ret = ERROR;
        return ret;
    }

    return ret;
}

//-- Check Exit conditions
static t_ret_code check_exit(unsigned int pawl_val)
{
    t_ret_code ret = RUN_AGAIN;

    switch(pawl_track.engaged_pawl)
    {
    case RIGHT:
        if (pawl_val <= RIGHT_THRES) {
            stopGearMotor();
            while (isMotorRunning());
            ret = COMPLETE;
        }
        break;

    case LEFT:
        if (pawl_val <= LEFT_THRES) {
            stopGearMotor();
            while (isMotorRunning());
            ret = COMPLETE;
        }
        break;

    case NONE:
        if ((int)pawl_val <= CAM_THRES_UPPER && (int)pawl_val >= CAM_THRES_LOWER) {
            stopGearMotor();

            if (move_cam) {
                pawl_track.CAM_DIR ^= FORWARD;
                startGearMotor(pawl_track.CAM_DIR, SLOW, 50);
                move_cam = 0;
            }

            ret = COMPLETE;
        }
        else
        {
            //-- If cam value is positive and dir is forward. Reverse
            if ((int) pawl_val < CAM_MID && pawl_track.CAM_DIR == BACKWARD || (int) pawl_val > CAM_MID && pawl_track.CAM_DIR == FORWARD) {
                stopGearMotor();

                //-- toggle direction
                pawl_track.CAM_DIR ^= FORWARD;

                V_PRINTF("GM_DIR_CH: %d\r\n", pawl_track.CAM_DIR);

                //-- Reverse gear motor
                startGearMotor(pawl_track.CAM_DIR, SLOW, 1000);
            }
        }
        break;
    default:
        ret = ERROR;
        break;
    }

    return ret;
}

//-- Turn on Motor
static t_ret_code turn_on_gmotor(void)
{
    t_ret_code ret = COMPLETE;
    unsigned char inc_motor;

    switch(pawl_track.engaged_pawl)
    {
    case RIGHT:
        inc_motor = CLOCKWISE;
        break;
    case LEFT:
        inc_motor = ANTICLOCKWISE;
        break;
    case NONE:
        inc_motor = REST;
        break;
    default:
        ret = ERROR;
        return ret;
    }

    if (!isGearMotorOn() && !isMotorRunning())
    {
        //-- If gear motor has timed out
        if (++pawl_track.tries.motor_inc > MAX_TRIES) {
            set_error(MAX_MOTOR_INC);
            ret =  ERROR;
        }

        if (inc_motor)
        {
            ret = incrementMainMotor((int)inc_motor, 5);
        }

        //-- Start gear motor again
        startGearMotor(pawl_track.CAM_DIR, MEDIUM, 200);
    }

    return ret;
}


//
//
//static t_ret_code disengageRight(unsigned int phase) {
//    unsigned int pawl_right;
//    int err;
//    unsigned int spi_tries = 0;
//
//    if (phase == INIT_PAWL) {
//        err = receive_hallsensors(NULL, NULL, &pawl_right);
//
//        //-- Error checking to see if we could receive pawl_right data
//        if (err < 0) {
//            set_error(INVALID_RIGHT_PAWL);
//            return ERROR;
//        }
//
//        if (pawl_right <= RIGHT_THRES) {
//            return COMPLETE;
//        }
//
//        //-- Keeps track of the number of times the main motor rotated to unlock the pawls
//        motor_inc_tries = 0;
//
//        //-- Turn the motor on until it reaches
//        startGearMotor(FORWARD, MEDIUM, 200);
//
//        return COMPLETE;
//
//    } else {    //-- RUN_PAWL
//
//        do{
//            err = receive_hallsensors(NULL, NULL, &pawl_right);
//            if (++spi_tries > MAX_TRIES) {
//                set_error(MAX_SPI_TRIES);
//                return ERROR;
//            }
//        } while (err);
//
//        if (pawl_right <= RIGHT_THRES) {
//            stopGearMotor();
//            while (isMotorRunning());
//            return COMPLETE;
//        }
//
//        if (!isGearMotorOn() && !isMotorRunning()) {
//            //-- If gear motor has timed out
//            if (++motor_inc_tries > MAX_TRIES) {
//                set_error(MAX_MOTOR_INC);
//                return ERROR;
//            }
//
//            incrementMainMotor(CLOCKWISE, 5);
//
//            //-- Start gear motor again
//            startGearMotor(FORWARD, MEDIUM, 200);
//        }
//
//        return RUN_AGAIN;
//    }
//}
//
//static t_ret_code disengageLeft(unsigned int phase) {
//    unsigned int pawl_left;
//    unsigned int spi_tries = 0;
//    int err;
//
//    if (phase == INIT_PAWL) {
//
//        err = receive_hallsensors(&pawl_left, NULL, NULL);
//
//        //-- Error checking to see if we could receive pawl_right data
//        if (err < 0) {
//            set_error(INVALID_LEFT_PAWL);
//            return ERROR;
//        }
//
//        if (pawl_left <= LEFT_THRES) {
//            //-- Already disengaged (Why?)
//            return COMPLETE;
//        }
//
//        motor_inc_tries = 0;
//
//        startGearMotor(BACKWARD, MEDIUM, 200);
//
//        return COMPLETE;
//    } else {    //-- RUN_PAWL
//
//        do{
//            err = receive_hallsensors(&pawl_left, NULL, NULL);
//            if (++spi_tries > MAX_TRIES) {
//                set_error(MAX_SPI_TRIES);
//                return ERROR;
//            }
//        } while (err);
//
//        if (pawl_left <= LEFT_THRES) {
//            stopGearMotor();
//            while (isMotorRunning());
//            return COMPLETE;
//        }
//
//        if (!isGearMotorOn() && !isMotorRunning()) {
//
//            //-- If gear motor has timed out
//            if (++motor_inc_tries > MAX_TRIES) {
//                set_error(MAX_MOTOR_INC);
//                return ERROR;
//            }
//
//            incrementMainMotor(ANTICLOCKWISE, 5);
//
//            //-- Start gear motor again
//            startGearMotor(BACKWARD, MEDIUM, 200);
//        }
//
//        return RUN_AGAIN;
//    }
//}
//
//t_ret_code engageBoth(unsigned int phase) {
//    int cam;
//    int err;
//    unsigned int spi_tries = 0;
//
//    if (phase == INIT_PAWL) {
//
//        //-- set direction to backward
//        dir = BACKWARD;
//
//        err = receive_hallsensors(NULL, &cam, NULL);
//
//        if (err < 0) {
//            set_error(INVALID_CAM);
//            return ERROR;
//        }
//
//        if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
//            //-- Already disengaged (Why?)
//            move_cam = 0;
//            return COMPLETE;
//        }
//
//        if (cam > CAM_MID) {
//            //-- Go backward
//            dir = BACKWARD;
//        } else {
//            //-- Go forward
//            dir = FORWARD;
//        }
//
//        motor_inc_tries = 0;
//
//        startGearMotor(dir, SLOW, 1000);
//
//        move_cam = 1;
//        return COMPLETE;
//
//    } else {    //-- RUN_PAWL
//
//        do{
//            err = receive_hallsensors(NULL, &cam, NULL);
//            if (++spi_tries > MAX_TRIES) {
//                set_error(MAX_SPI_TRIES);
//                return ERROR;
//            }
//        } while (err == -1);
//
//        if (cam <= CAM_THRES_UPPER && cam >= CAM_THRES_LOWER) {
//            stopGearMotor();
//
//            //-- Move in the reverse direction to counter-act inertia only if
//            if (move_cam) {
//                dir ^= FORWARD;
//                startGearMotor(dir, SLOW, 50);
//                move_cam = 0;
//            }
//
//            return COMPLETE;
//        }
//
//        //-- If cam value is positive and dir is forward. Reverse
//        if (cam < CAM_MID && dir == BACKWARD || cam > CAM_MID && dir == FORWARD) {
//            stopGearMotor();
//
//            //-- toggle direction
//            dir ^= FORWARD;
//            V_PRINTF("GM_DIR_CH: %d\r\n", dir);
//            //-- Reverse gear motor
//            startGearMotor(dir, SLOW, 1000);
//        }
//
//
//        if (!isGearMotorOn()) {
//            if (++motor_inc_tries > MAX_TRIES) {
//                set_error(MAX_MOTOR_INC);
//                return ERROR;
//            }
//
//            startGearMotor(dir, SLOW, 1000);
//
//            V_PRINTF("GM_DIR: %d\r\n", dir);
//        }
//
//        return RUN_AGAIN;
//    }
//}
//
//
