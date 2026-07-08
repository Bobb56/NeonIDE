#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "state.h"

int console_handle_key(struct estate *state, short k);
void start_using_console(char* name);


#endif