/*
 * motor.h
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */

#ifndef MOTOR_H_
#define MOTOR_H_


#define STEP BIT4;
#define DIR BIT2;

//-- Direction definitions
#define REST 0
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

//-- Counts
#define UPPER_COUNT 1000
#define MID_COUNT 500


extern int cur_direction;

//-- Initializes main motor functionality and interrupts
void init_Main_Motor(void);

//-- Moves the Main Motor either clockwise or anti-clockwise depending on the global state
int startMainMotor(int increment);

#endif /* MOTOR_H_ */
