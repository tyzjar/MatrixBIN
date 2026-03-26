#ifndef COMMAND_H
#define COMMAND_H

#define MAX_LINE 10000
#define MAX_COMMANDS 10
#define MAX_ARGS 100

#include <regex.h>

// Struct to contain parsed input
static struct __commands
{
    int num_commands;
//    int num_args[MAX_COMMANDS]; // Number of args for each command
    char* command_list[MAX_COMMANDS]; // Contains the programs to be run
    regex_t reg_list[MAX_COMMANDS]; // The args for each command
    void (*pfuncArr[MAX_COMMANDS])(char *, int);
} commands = {-1};

// 0 - success
// 1 - error

int register_command(const char *command, void (*pfunc)(char *, int)); // arg for getopt function
int parse_command(const char* input, int sockfd);


#endif // COMMAND_H
