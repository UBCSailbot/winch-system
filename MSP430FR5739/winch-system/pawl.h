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
#define RIGHT_THRES 10000
#define LEFT_THRES  13000
#define CAM_THRES_UPPER 3500
#define CAM_THRES_LOWER -3500

#define CAM_OFFSET  14781
#define CAM_TIMEOUT 50

#define MAX_TRIES 3

//-- Pawl control Phases
#define INIT_PAWL   0
#define RUN_PAWL    1

//-- Move the main pawls depending on the cur_direction
int move_pawl(unsigned int direction, unsigned int phase);

//-- Disengages right pawl by controlling gear motor
static int disengageRight(unsigned int phase);

//-- Disengages left pawl by controlling gear motor
static int disengageLeft(unsigned int phase);

//-- Disengages both pawls by controlling gear motor
static int disengageBoth(unsigned int phase);

#endif /* PAWL_H_ */
