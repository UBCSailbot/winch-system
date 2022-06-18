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

//-- Counts - 176 Hz PWM
#define UPPER_COUNT 5682
#define MID_COUNT 2841

#define POT_SCALAR 11
#define MAX_MOTOR_TRIES 3

//-- Two phase motor control
#define INIT_MMOTOR 0
#define RUN_MMOTOR  1


//-- State of motor
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
int setMainMotorPosition(unsigned int phase);

//-- Increment motor by a certain amount
int incrementMainMotor(int direction, int increment);

//-- Hault motor operation
void stopMainMotor(void);

//-- Turns on power to the motor
void turnOnMotor(void);

//-- Turns off power to the motor
void turnOffMotor(void);

//-- Checks if the motor is On
int isMotorOn(void);

// Sets the current motor_stat.position value, Error if calculated value not between 0-360
int setCurrentPosition();

// Get saved position value from the last call to getCurrentPosition
unsigned int getCurrentCachedPosition(void);

// Calculates which direction to move and sets the value to the motor state
unsigned int setDirectionToMove(unsigned int setpoint);

// Get saved direction value from the last call to setDirectionToMove
unsigned int getCurrentCachedDirection(void);

//-- FOR TEST
int moveToPosition(unsigned int setpoint);

#endif /* MOTOR_H_ */
