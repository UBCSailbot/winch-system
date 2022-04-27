/*
 * commands.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stddef.h>

#define ACTIVE_CMD_SIZE 4

//-- command ids
#define SET_POS         1
#define QUERY_POS       2
#define STOPLOCK        4
#define ALIVE           8
#define UNDEF           16
#define ACTION_BUSY     32


typedef struct cmd {
    int type;
    int cont_state;
    unsigned int data1;     // SetPoint
    unsigned int data2;     // Direction
    unsigned int msg;       // UCCM MSG

}t_cmd;

static int active_cmd;
static int cmd_index = 0;
static t_cmd cmd_list[ACTIVE_CMD_SIZE];


//-- Creates a new command
t_cmd * new_command(int cmd, unsigned int data1, unsigned int data2, unsigned int uccm_msg);

//-- Removes current command from the list
void end_command(void);

//-- Returns 1 if available and 0 otherwise
unsigned int is_command_available(void);

//-- Returns 1 if not busy and 0 otherwise
unsigned int is_busy(int cmd_id);

//-- Indicates if the max number of active commands have been reached
unsigned int max_active_reached(void);

//-- GETERS and SETTERS --

//-- Saves the current state of the command in its structure
void save_current_state(unsigned int state);

//-- Set the values for the data variable in the command structure
void set_uccm_msg(unsigned int uccm_msg);

//-- Returns the command currently running
static t_cmd * get_current_command(void);


//-- SOURCE CODE --

t_cmd * new_command(int cmd_id, unsigned int data1, unsigned int data2, unsigned int uccm_msg) {
    t_cmd* new_cmd;

    if (cmd_index < ACTIVE_CMD_SIZE - 1) {
        cmd_index++;
        new_cmd = &cmd_list[cmd_index];
        new_cmd->cont_state = DECODE;

        if (is_busy(cmd_id)) {
            //-- The command is already active
            new_cmd->type = ACTION_BUSY;
            active_cmd |= ACTION_BUSY;
            new_cmd->msg = 0x1;
        } else {
            new_cmd->data1 = data1;
            new_cmd->data2 = data2;
            new_cmd->type = cmd_id;
            new_cmd->msg = uccm_msg;
            active_cmd |= cmd_id;
        }

        return new_cmd;
    } else return (t_cmd*)0;
}

void end_command(void) {
    if (cmd_index == -1) return;

    cmd_index--;
}

unsigned int is_command_available(void) {
    return !(cmd_index < 0);
}

void save_current_state(unsigned int state) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {
        cur_cmd->cont_state = state;
    }
}

void set_data(int data1, int data2) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {
        cur_cmd->data1 = data1;
        cur_cmd->data2 = data2;
    }
}

unsigned int is_busy(int cmd_id) {
    return active_cmd & cmd_id;
}

unsigned int max_active_reached(void) {
    return cmd_index >= ACTIVE_CMD_SIZE - 1;
}

void set_uccm_msg(unsigned int uccm_msg) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {
        cur_cmd->msg = uccm_msg;
    }
}

static t_cmd * get_current_command(void) {
    if (cmd_index < 0) return (t_cmd*)0;
    return &cmd_list[cmd_index];
}




#endif /* COMMANDS_H_ */
