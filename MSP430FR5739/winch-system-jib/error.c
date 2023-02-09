/*
 * error.c
 *
 *  Created on: Feb 4, 2023
 *      Author: mlokh
 */
#include "error.h"


/**
 *  Name:       set_error
 *
 *
 *  Purpose:    capture the most recent error code
 *
 *  Params:     error_code_t - error code enum value
 *
 *  Return:     none
 *
 *  Notes:      increments error count. if error count > MAX_
 *              ERROR_COUNT then max_error_reached is set which
 *              would cause a software reset
 */
void set_error(error_code_t error)
{
    static unsigned int error_count = 0;

    error_state.error = error_state.error | ((unsigned long)error << error_state.byte_offset * 8);

    error_state.byte_offset++;

    if (++error_count >= MAX_ERROR_COUNT)
    {
        error_state.max_error_reached = 1;
    }
}

/**
 *  Name:       get_error
 *
 *
 *  Purpose:    return the most recent error code previously captured
 *
 *  Params:     none
 *
 *  Return:     unsigned long - error code 32 bit
 *
 *  Notes:      error cleared after returned
 */
unsigned long get_error(void)
{
    unsigned long tmp_err = error_state.error;
    error_state.error = NO_ERROR;
    error_state.byte_offset = 0;
    return tmp_err;
}

/**
 *  Name:       max_error_reached
 *
 *
 *  Purpose:    has the max error value reached
 *
 *  Params:     none
 *
 *  Return:     1 - max error count reached
 *              0 - max error count not reached
 *
 *  Notes:      flag is cleared after read
 */
unsigned char max_error_reached(void)
{
    unsigned char tmp_max_error_reached = error_state.max_error_reached;
    error_state.max_error_reached = 0;
    error_state.error_count = 0;
    return tmp_max_error_reached;
}
