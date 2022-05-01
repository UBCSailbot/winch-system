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
#define ROTATE_CW 3

//-- Gear motor direction
#define BACKWARD   0
#define FORWARD    1

//-- Pawl threshold values
#define RIGHT_THRES 0xfe50
#define LEFT_THRES  0xef90
#define CAM_THRES_UPPER 200
#define CAM_THRES_LOWER -500

#define CAM_MID -300
#define CAM_TIMEOUT 50

#define MAX_TRIES 3

extern int cur_direction;
extern int GearMotorOn;

//-- Move the main pawls depending on the cur_direction
int move_pawl(void);

//-- Disengages right pawl by controlling gear motor
static int disengageRight(void);

//-- Disengages left pawl by controlling gear motor
static int disengageLeft(void);

//-- Disengages both pawls by controlling gear motor
static int disengageBoth(void);

static int test_func(void);

#endif /* PAWL_H_ */
