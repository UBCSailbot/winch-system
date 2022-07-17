/*
 * commands.c
 *
 *  Created on: Jul. 16, 2022
 *      Author: mlokh
 */
#include "commands.h"

void add_new_command(unsigned int rx_msg) {
    t_cmd * new_cmd = (t_cmd *)(0);

    if (num_active_cmd < ACTIVE_CMD_SIZE) {

        for (int i = 0; i <= num_active_cmd; i++) {
            if ( is_cmd_index_free(i) ) {
                new_cmd = &cmd_list[i];
            }
        }

        if (new_cmd == (t_cmd *)(0)) {
            //-- no free space found for command
            return;
        }

        new_cmd->type = PREMATURE;
        new_cmd->state = DECODE;
        new_cmd->rx_msg = rx_msg;
        new_cmd->tx_msg = 0;

        //-- Not free anymore (cannot be used by another command)
        new_cmd->free = 0;

        num_active_cmd++;
    }
}

void end_command(void) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {
        //-- Clear current command from active
        active_cmd &= ~cur_cmd->type;
        num_active_cmd--;

        //-- space is free to use by any other command
        cur_cmd->free = 1;
    }
}

void set_current_command(unsigned int cmd, unsigned int tx_msg) {
    t_cmd * current_cmd;
    if (is_busy(cmd)) {
        cmd = ACTION_BUSY;
        tx_msg = BUSY_MSG << 9;
    }

    current_cmd = get_current_command();

    if (current_cmd == (t_cmd*)0) {
        //-- No current command available
        return
    }

    current_cmd->type = cmd;
    current_cmd->state = lookup_cmd_start_state(cmd);
    current_cmd->tx_msg = tx_msg;

}

static enum States lookup_cmd_start_state(unsigned int cmd) {
    enum States start_state = IDLE;

    switch(cmd) {

    case SET_POS:
        start_state = TURN_MOTOR_ON;
        break;

    case QUERY_POS:
    case ALIVE:
    case UNDEF:
    case ACTION_BUSY:
        start_state = SEND_TO_UCCM;
        break;

    case IDLE_CMD:
        // Never want to set the command to IDLE (only set when no commands are running), abort instead

    case STOPLOCK:
    default:
        start_state = ABORT;
        break;

    }

    return start_state;
}

enum States get_current_command_state(void) {
    t_cmd * current_cmd;
    enum States current_state;

    if (is_command_available()) {
        //-- Want to keep processing the next command each time we get the state value
        inc_current_command_index();

        current_cmd = get_current_command();
        current_state = current_cmd->state;
    } else {
        current_state = IDLE;
    }

    return current_state;
}

unsigned int get_current_rx_msg(void) {
    t_cmd * current_cmd;
    unsigned int current_rx_msg;

    if (is_command_available()) {
        current_cmd = get_current_command();
        current_rx_msg = current_cmd->rx_msg;
    }

    return current_rx_msg;
}

static unsigned int is_command_available(void) {
    return !(num_active_cmd <= 0);
}

void save_current_state(unsigned int state) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {
        cur_cmd->cont_state = state;
    }
}

unsigned int is_busy(int cmd_id) {
    return active_cmd & cmd_id;
}

unsigned int max_active_reached(void) {
    return num_active_cmd >= ACTIVE_CMD_SIZE;
}

void set_uccm_msg(unsigned int uccm_msg) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {
        cur_cmd->msg = uccm_msg;
    }
}

void clear_all_commands(void) {

    //-- minus one indicates if the list active list is empty
    cmd_index = -1;

    //-- Reset active command tracker
    active_cmd = 0;
}

static t_cmd * get_current_command(void) {
    if (cmd_index < 0) return (t_cmd*)0;
    return &cmd_list[cmd_index];
}

static void inc_current_command_index(void) {
    int count = 0;
    unsigned int index = cmd_index;

    //-- Only if there are active commands
    if (is_command_available()) {
        do {
            index = (++index) % num_active_cmd;
            count++;
        } while ( count < num_active_cmd && !is_cmd_index_free(index) );


        if (count == num_active_cmd) {
            //-- if we have gone through all possible commands leave cmd_index to be the same
            index = cmd_index;
        }

    }

    cmd_index = index;
}

static unsigned int is_cmd_index_free(unsigned index) {
    if (index < ACTIVE_CMD_SIZE) {
        return cmd_list[index].free;
    }
    else return 0;
}




