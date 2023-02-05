/*
 * error.h
 *
 *  Created on: Feb 4, 2023
 *      Author: mlokh
 */

#ifndef ERROR_H_
#define ERROR_H_

#define MAX_ERROR_COUNT 5

typedef enum error_code {
    NO_ERROR,
    MAX_ERROR_VAL
}error_code_t;

typedef struct error_state_struct {
    error_code_t error;
    unsigned int error_count;
    unsigned char max_error_reached;
} error_state_t;

static error_state_t error_state;


// Set error code to error_state struct
void set_error(error_code_t error);

// Getter for error code in error_state struct
error_code_t get_error(void);

// Getter for max_error_reached in error_state struct
unsigned char max_error_reached(void);


#endif /* ERROR_H_ */
