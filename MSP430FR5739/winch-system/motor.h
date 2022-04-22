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

//-- Port P1.0
#define ON_MOTOR BIT0

//-- Direction definitions
#define REST 0
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

//-- Counts - 176 Hz PWM
#define UPPER_COUNT 5682
#define MID_COUNT 2841

#define POT_SCALAR 11
#define MAX_MOTOR_TRIES 3


//-- Initializes main motor functionality and interrupts
void init_Main_Motor(void);

//-- Moves the Main Motor either clockwise or anti-clockwise to a specified position (0 - 360)
int setMainMotorPosition(int position);

//-- Increment motor by a certain amount
int incrementMainMotor(int direction, int increment);

//-- Haults motor operation
void stopMainMotor();

#endif /* MOTOR_H_ */
