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
#define BACKWARD   0
#define FORWARD    1

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

//-- Move the main pawls depending on the cur_direction
t_ret_code move_pawl(unsigned int phase);

//-- Disengages right pawl by controlling gear motor
static t_ret_code disengageRight(unsigned int phase);

//-- Disengages left pawl by controlling gear motor
static t_ret_code disengageLeft(unsigned int phase);

//-- Disengages both pawls by controlling gear motor
t_ret_code engageBoth(unsigned int phase);

#endif /* PAWL_H_ */
