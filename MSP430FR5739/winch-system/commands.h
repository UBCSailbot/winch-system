/*
 * commands.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#define ACTIVE_CMD_SIZE 1

//-- command ids
#define SET_POS    1
#define QUERY_POS  2
#define STOPLOCK   4
#define RESPONSIVE 8
#define UNDEF      16
#define BUSY       32


typedef struct cmd {
    int type;
    int cont_state;
    unsigned int data;

}t_cmd;

static int active_cmds;
static int cmd_index = 0;
static t_cmd cmd_list[ACTIVE_CMD_SIZE];


//-- Creates a new command
t_cmd * new_command(int cmd);

//-- Removes current command from the list
void end_command(void);

//-- Returns the command currently running
t_cmd * get_current_command(void);

//-- Returns 1 if available and 0 otherwise
int is_command_available(void);


//-- SOURCE CODE --

t_cmd * new_command(int cmd) {
    t_cmd* new_cmd;
    t_cmd * curr_cmd;

    if (cmd_index >= 0) {
        //-- Save the state so we know where to continue
        curr_cmd = &cmd_list[cmd_index];
        curr_cmd->cont_state = state;
    }

    cmd_index++;

    if (cmd_index < ACTIVE_CMD_SIZE) {
        new_cmd = &cmd_list[cmd_index];
        new_cmd->cont_state = DECODE;
        new_cmd->type = cmd;
        active_cmds |= cmd;
        return new_cmd;
    } else return (t_cmd*)0;
}

void end_command(void) {
    if (cmd_index == -1) return;

    cmd_index--;
}

t_cmd * get_current_command(void) {
    if (cmd_index < 0) return (t_cmd*)0;
    return &cmd_list[cmd_index];
}

int is_command_available(void) {
    return !(cmd_index < 0);
}






#endif /* COMMANDS_H_ */
