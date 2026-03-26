#include "command.h"
#include <regex.h>
#include <string.h>
#include <stdlib.h>

//static struct __commands
//{
//    int num_commands;
//    int num_args[MAX_COMMANDS]; // Number of args for each command
//    char* command_list[MAX_COMMANDS]; // Contains the programs to be run
//    regex_t* reg_list[MAX_COMMANDS]; // The args for each command
//    void (*pfuncArr[MAX_COMMANDS])(char *, int);
//} commands = {0};

int register_command(const char *command, void (*pfunc)(char *, int))
{
    if(command == 0 || pfunc == 0)
        return 1;

    commands.num_commands++;

    if (regcomp(&commands.reg_list[commands.num_commands], command, REG_EXTENDED|REG_NOSUB) != 0) {
          commands.num_commands--;
          return 1;
    }

    commands.command_list[commands.num_commands] = malloc(sizeof(strlen(command)));
    strcpy(commands.command_list[commands.num_commands], command);
    commands.pfuncArr[commands.num_commands] = pfunc;
    return 0;
}

char *choppy(const char* s)
{
    char *n = malloc( strlen( s ? s : "\n" ) );
    if( s )
        strcpy( n, s );
    n[strlen(n)-1]='\0';
    return n;
}

int parse_command(const char *input, int sockfd)
{
    int i;
    int reti;

    char* command = choppy(input);

    for(i = 0; i <= commands.num_commands; i++) {
        reti = regexec(&commands.reg_list[i], command, 0, 0, 0);
        if(!reti){
            (*commands.pfuncArr[i])(input, sockfd);
            return 0;
        }
    }

    return 1;
}
