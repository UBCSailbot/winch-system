/*
 * motor.h
 *
 *  Created on: Apr. 16, 2022
 *      Author: mlokh
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "return_codes.h"
#include "spi.h"


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

//-- Counts - 352 Hz PWM
#define UPPER_COUNT_FAST 2842
#define MID_COUNT_FAST 1421

//-- Counts - 176 Hz PWM
#define UPPER_COUNT_MID 5682
#define MID_COUNT_MID 2841

////-- Counts - 44 Hz PWM
#define UPPER_COUNT_SLOW 22727
#define MID_COUNT_SLOW 11364

//-- Counts - 22 Hz PWM
#define UPPER_COUNT_SUPER_SLOW 45454
#define MID_COUNT_SUPER_SLOW 22727

//-- Ramping Params
#define UPPER_COUNT_MIN UPPER_COUNT_MID
#define UPPER_COUNT_MAX UPPER_COUNT_SLOW
#define UPPER_COUNT_DEC 355
#define UPPER_COUNT_INC 711

//-- Motor fault tracking
#define POS_FAULT_LIMIT     3

//-- Each rotation of the motor is +410 mV
//-- Range 820 to 3278 therefore POT_SCALAR = (POT_MAX_VALUE - POT_MIN_VALUE)/360
#define POT_SCALAR 6
#define POT_OFFSET POT_MIN_VALUE

#define MAX_MOTOR_TRIES 3

//-- Two phase motor control
#define INIT_MMOTOR 0
#define RUN_MMOTOR  1

//-- FUNCTION MACROS
#define CALC_POS(X) ( (X - POT_OFFSET) / POT_SCALAR )
#define CALC_VOLT(X) ( (X * POT_SCALAR) + POT_OFFSET )

//-- The number of steps until checking motor status
#define STEP_COUNT_FOR_MOTOR_CHECK  220
#define EXPECTED_POS_DIFF     8


typedef enum motor_speed {
    MMOTOR_FAST,
    MMOTOR_MID,
    MMOTOR_SLOW,
    MMOTOR_SUPER_SLOW
} motor_speed_t;

//-- State of the main motor
typedef struct motor_status_struct {
    volatile unsigned int power;
    unsigned int position;
    unsigned int direction;
    unsigned int setpoint;
    volatile unsigned char motor_inc_stat;
    volatile unsigned char accel;
    volatile unsigned char running;
} motor_stat_t;

//-- Tracks the main motor rotations for fault detection
typedef struct motor_tracker_struct {
    unsigned int steps;
    volatile unsigned int last_position;
    volatile unsigned char fault;
} motor_tracker_t;

static motor_stat_t motor_stat;
static motor_tracker_t motor_tracker;


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

// Checks if a fault occurs and clears it
unsigned char checkMotorFaultAndClear(void);

#endif /* MOTOR_H_ */
