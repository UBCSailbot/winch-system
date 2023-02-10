/*
 * error.h
 *
 *  Created on: Feb 4, 2023
 *      Author: mlokh
 */

#ifndef ERROR_H_
#define ERROR_H_

#define MAX_ERROR_COUNT 5

#define BYTES_OFFSET_MASK   0x00000003
#define MAX_CNTFLG_MASK     0x00000004
#define ERROR_CNT_MASK      0x000000F8
#define ERROR_MASK          0xFFFFFF00

#define BYTES_OFFSET_OFFSET 0
#define MAX_CNTFLG_OFFSET   2
#define ERROR_CNT_OFFSET    3
#define ERROR_OFFSET        8

typedef enum error_code {
    NO_ERROR,
    /* motor.c [1 - 8] */
    MOTOR_NOT_ON,       // 1
    INVALID_DIR,
    INVALID_SETPOINT,
    MMOTOR_FAULT,
    SET_DIR_TO_MOVE_ERROR,  // 5
    RECEIVE_POT_ERROR,
    CURR_POSITION_EXCEED_360,
    SET_CURRENT_POS_ERROR,

    /* gearmotor.c */

    /* pawl.c [9 - 14] */
    MOTOR_NFAULT,
    INVALID_RIGHT_PAWL,     // 10
    INVALID_LEFT_PAWL,
    INVALID_CAM,
    MAX_SPI_TRIES,
    MAX_MOTOR_INC,

    /* spi.c [15 - 19] */
    CONF_HALL_AIN0_ERROR, // 15
    CONF_HALL_AIN1_ERROR,
    CONF_HALL_AIN2_ERROR,
    INVALID_POT,
    MAX_CONF_HALL_ATTEMPTS,

    /* statemachine.c [20] */
    INVALID_RX_SETPOINT,       // 20

    MAX_ERROR_VAL
}error_code_t;

static unsigned long error_header;

typedef struct error_state_struct {
    unsigned long  error;
    unsigned int error_count;
    unsigned char max_error_reached;
    unsigned char byte_offset;
} error_state_t;

static error_state_t error_state;


// Set error code to error_state struct
void set_error(error_code_t error);

// Getter for error code in error_state struct
unsigned long get_error(void);

// Getter for max_error_reached in error_state struct
unsigned char max_error_reached(void);


#endif /* ERROR_H_ */
