#ifndef READ_CONFIG_H
#define READ_CONFIG_H

#include "devdrvd.h"

enum CONFIG_ERROR {
    CESUCCESS = 0,   // no error
    CENOSCALETYPE,   // type not given for scale
    CEGPIORANGE,     // range is wrong in config file
    CENOGPIODEVICE,  // no in or out specified for gpio connection    
    CEREGCOMP,       // regex pattern compile error o_O
    CEMALLOC
};

extern const char *config_errors[];

int read_config(const char*);

int get_ipadress(char**);
int get_port(int*);

int get_sections(struct devdrvd_connect***, int*);
void free_sections(struct devdrvd_connect***, int);

int free_config();

int errline();

#endif
