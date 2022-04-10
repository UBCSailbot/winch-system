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

//-- Pawl threshold values
#define RIGHT_THRES 3250
#define LEFT_THRES 6000
#define CAM_THRES 200 //???

#define MAX_TRIES 10

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

#endif /* PAWL_H_ */
