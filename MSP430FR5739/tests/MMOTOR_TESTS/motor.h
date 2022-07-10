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

#define OFF 0
#define ON 1

//-- Direction definitions
#define REST 0
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

//-- Counts - 88 Hz PWM
#define UPPER_COUNT 11364
#define MID_COUNT 5682

//-- Range 410 to 3685 therefore POT_SCALAR = (3685 - 410)/360
#define POT_SCALAR 9
#define POT_OFFSET 410
#define MAX_MOTOR_TRIES 3

//-- FUNCTION MACROS
#define CALC_POS(X) ( (X - POT_OFFSET) / POT_SCALAR )
#define CALC_VOLT(X) ( (X * POT_SCALAR) + POT_OFFSET )

//-- Initializes main motor functionality and interrupts
void init_Main_Motor(void);

//-- Moves the Main Motor either clockwise or anti-clockwise to a specified position (0 - 360)
int setMainMotorPosition(int position);

//-- Increment motor by a certain amount
int incrementMainMotor(int direction, int increment);

//-- Hault motor operation
void stopMainMotor(void);

int isMotorOn(void);

// Returns negative if error otherwise position between 0-360
unsigned int getCurrentPosition(void);

#endif /* MOTOR_H_ */
