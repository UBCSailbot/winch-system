/*
 * motor.h
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "return_codes.h"


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

//-- Counts - 352 Hz PWM
#define UPPER_COUNT_FAST 2842
#define MID_COUNT_FAST 1421

//-- Counts - 176 Hz PWM
#define UPPER_COUNT_MID 5682
#define MID_COUNT_MID 2841

////-- 44 Hz PWM
#define UPPER_COUNT_SLOW 22727
#define MID_COUNT_SLOW 11364

//-- Range 410 to 3685 therefore POT_SCALAR = (3685 - 410)/360
#define POT_SCALAR 9
#define POT_OFFSET 410

#define MAX_MOTOR_TRIES 3

//-- Two phase motor control
#define INIT_MMOTOR 0
#define RUN_MMOTOR  1

//-- FUNCTION MACROS
#define CALC_POS(X) ( (X - POT_OFFSET) / POT_SCALAR )
#define CALC_VOLT(X) ( (X * POT_SCALAR) + POT_OFFSET )

typedef enum motor_speed {
    MMOTOR_FAST,
    MMOTOR_MID,
    MMOTOR_SLOW
} motor_speed_t;

//-- State of the main motor
typedef struct motor_status_struct {
    volatile unsigned int power;
    unsigned int position;
    unsigned int direction;
    unsigned int setpoint;
} motor_stat_t;

motor_stat_t motor_stat;


//-- Initializes main motor functionality and interrupts
void init_Main_Motor(void);

//-- Moves the Main Motor either clockwise or anti-clockwise to a specified position (0 - 360)
t_ret_code setMainMotorPosition(unsigned int phase);

//-- Increment motor by a certain amount
int incrementMainMotor(int direction, int increment);

//-- Start motor operation
static void startMainMotor(void);

//-- Halt motor operation
void stopMainMotor(void);

//-- Enables power to motor
void turnOnMotor(void);

//-- Disables power to motor
void turnOffMotor(void);

//-- Checks if there is power to motor
int isMotorOn(void);

//-- Checks if motor is in operation
int isMotorRunning(void);

// Sets the current motor_stat.position value, Error if calculated value not between 0-360
int setCurrentPosition();

// Get saved position value from the last call to getCurrentPosition
unsigned int getCurrentCachedPosition(void);

// Calculates which direction to move and sets the value to the motor state
int setDirectionToMove(unsigned int setpoint);

// Get saved direction value from the last call to setDirectionToMove
unsigned int getCurrentCachedDirectionToMove(void);

// Sets the speed of the motor by changing the timer counts
void setMotorSpeed(motor_speed_t speed_sel);


#endif /* MOTOR_H_ */
