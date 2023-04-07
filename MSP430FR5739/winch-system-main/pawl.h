/*
 * pawl.h
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#ifndef PAWL_H_
#define PAWL_H_

#include "return_codes.h"

//-- Direction definitions
#define REST 0
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

//-- Gear motor direction
typedef enum cam_dir_type{
    BACKWARD,
    FORWARD
}cam_dir_type_t;

//-- Types of Pawls
typedef enum pawl_type{
    RIGHT,
    LEFT,
    NONE
}pawl_type_t;

//-- Pawl action states
typedef enum pawl_action_states{
    GET_SPI,
    UPDATE_DIR,
    CHECK_EXIT_COND,
    RUN_GMOTOR,
    END_ACTION,
    SEND_ERROR,
    MAX_STATE_NUM
}t_pawl_state;

//-- Pawl threshold values
#define RIGHT_THRES 0xfb00
#define LEFT_THRES  0xf580
#define CAM_THRES_UPPER -200
#define CAM_THRES_LOWER -400

#define CAM_MID -300
#define CAM_TIMEOUT 50

#define MAX_TRIES 3

//-- Pawl control Phases
#define INIT_PAWL   0
#define RUN_PAWL    1

typedef struct tries_counter {
    unsigned int motor_inc;
    unsigned int read_spi;
}tries_counter_t;

typedef struct pawl_tracker {
    pawl_type_t engaged_pawl;
    cam_dir_type_t CAM_DIR;
    tries_counter_t tries;
}pawl_tracker_t;

static pawl_tracker_t pawl_track;

//-- Move the main pawls depending on the cur_direction
t_ret_code move_pawl(unsigned int phase);

//-- Setup the pawl values
static t_ret_code setup_pawl(void);

//-- Perform action on the pawls
static t_ret_code pawl_action(void);

//-- Get SPI Function
static t_ret_code get_spi(unsigned int *pawl_val);

//-- Update DIR function
static t_ret_code update_dir(unsigned int pawl_val);

//-- Check exit cond
static t_ret_code check_exit(unsigned int pawl_val);

//-- Turning gmotor back on
static t_ret_code turn_on_gmotor(void);


////-- Disengages right pawl by controlling gear motor
//static t_ret_code disengageRight(unsigned int phase);
//
////-- Disengages left pawl by controlling gear motor
//static t_ret_code disengageLeft(unsigned int phase);
//
////-- Disengages both pawls by controlling gear motor
//t_ret_code engageBoth(unsigned int phase);



#endif /* PAWL_H_ */
