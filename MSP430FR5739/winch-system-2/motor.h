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

//-- Range 410 to 3685 therefore POT_SCALAR = (3685 - 410)/360
#define POT_SCALAR 9
#define POT_OFFSET 410

#define MAX_MOTOR_TRIES 3

//-- Two phase motor control
#define INIT_MMOTOR 0
#define RUN_MMOTOR  1


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
int setMainMotorPosition(unsigned int position, unsigned int * dir, unsigned int phase);

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

// Returns negative if error. position value will be between 0-360
int getCurrentPosition(unsigned int * position);

// Calculates the direction to rotate the motor
unsigned int getDirection(unsigned int position, unsigned int * dir);

#endif /* MOTOR_H_ */
