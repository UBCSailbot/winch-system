/*
 * gearmotor.h
 *
 *  Created on: Apr. 4, 2022
 *      Author: mlokh
 */

#ifndef GEARMOTOR_H_
#define GEARMOTOR_H_

//-- Port J
#define NFAULT BIT3
#define MODE BIT2
#define PHASE BIT1
#define NSLEEP BIT0

//-- Port 1
#define ENABLE BIT4

//-- Motor Speed
#define FAST        /* 90% duty */
#define MEDIUM      /* 50% duty */
#define SLOW        /* 20% duty */


//-- Initializes the gear motor - PWM and timeout
void init_gearmotor(void);

//-- Starts the gear motor to either forward (1) or backward (0) direction
void startGearMotor(int forward, int speed, int timeout);

//-- Stops the gear motor from moving
void stopGearMotor(void);




#endif /* GEARMOTOR_H_ */
