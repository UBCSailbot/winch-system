/*
 * return_codes.h
 *
 *  Created on: Aug. 1, 2022
 *      Author: mlokh
 */

#ifndef RETURN_CODES_H_
#define RETURN_CODES_H_

typedef enum Ctrl_State {
    RUN_AGAIN,
    COMPLETE,
    ERROR,
    RESTART,

    MAX_RET_CODE
}t_ret_code;

#endif /* RETURN_CODES_H_ */
