/*
 * statemachine.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

//-- StateMachine states
#define WAIT        0
#define DECODE      1

#define PAWL  2
#define MOTOR 3

#define READ_POT    4
#define HAULT       5

#define NOTIFY_UCCM


//-- command ids
#define SET_POS    1
#define QUERY_POS  2
#define STOPLOCK   4
#define RESPONSIVE 8
#define UNDEF      16
#define BUSY       32


void handle_state(void);

int decode_msg(char msg[2], int * data);


#endif /* STATEMACHINE_H_ */
