/*
 * pawl.h
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#ifndef PAWL_H_
#define PAWL_H_

//-- Direction definitions
#define REST 0
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

//-- Gear motor direction
#define BACKWARD   0
#define FORWARD    1

//-- Pawl threshold values
#define RIGHT_THRES 0xef00
#define LEFT_THRES  0xed00
#define CAM_THRES_UPPER -200
#define CAM_THRES_LOWER -400

#define CAM_MID -300
#define CAM_TIMEOUT 50

#define MAX_TRIES 3

//-- Pawl control Phases
#define INIT_PAWL   0
#define RUN_PAWL    1

//-- Move the main pawls depending on the cur_direction
int move_pawl(unsigned int phase);

//-- Disengages right pawl by controlling gear motor
static int disengageRight(unsigned int phase);

//-- Disengages left pawl by controlling gear motor
static int disengageLeft(unsigned int phase);

//-- Disengages both pawls by controlling gear motor
int engageBoth(unsigned int phase);

#endif /* PAWL_H_ */
