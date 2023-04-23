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

//-- Port P3.3
#define ON_MOTOR BIT3

#define OFF 0
#define ON 1

//-- Direction definitions
#define REST 0
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

//-- Counts - 88 Hz PWM
#define UPPER_COUNT 5682
#define MID_COUNT 2841

//-- Counts - 352 Hz PWM
#define UPPER_COUNT_FAST 2842
#define MID_COUNT_FAST 1421

//-- Counts - 176 Hz PWM
#define UPPER_COUNT_MID 5682
#define MID_COUNT_MID 2841

////-- 44 Hz PWM
#define UPPER_COUNT_SLOW 22727
#define MID_COUNT_SLOW 11364

//-- Counts - 22 Hz PWM
#define UPPER_COUNT_SUPER_SLOW 45454
#define MID_COUNT_SUPER_SLOW 22727

//-- Counts - 11 Hz PWM
#define UPPER_COUNT_SUPER_SUPER_SLOW 90908
#define MID_COUNT_SUPER_SUPER_SLOW 45454

//-- Range 410 to 3685 therefore POT_SCALAR = (3685 - 410)/360
#define POT_SCALAR 9
#define POT_OFFSET 410
#define MAX_MOTOR_TRIES 3

//-- FUNCTION MACROS
#define CALC_POS(X) ( (X - POT_OFFSET) / POT_SCALAR )
#define CALC_VOLT(X) ( (X * POT_SCALAR) + POT_OFFSET )

//-- The number of steps until checking motor status
#define STEP_COUNT_FOR_MOTOR_CHECK  220

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

unsigned int checkMotorFault(void);

#endif /* MOTOR_H_ */
