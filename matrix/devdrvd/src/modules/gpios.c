#include "gpios.h"

#include "../devdrvd.h"
#include "../read_config.h"

#include <regex.h>
#include <stdlib.h>
#include <confuse.h>

const char *gpio_pattern =  "^[0-9]{1,3}\\:[0-9]{1,3}$";

int read_gpio_config(struct cfg_t * cfg, struct devdrvd_connect *connect)
{
    regex_t regex;
    int reti;
    int return_code = CESUCCESS;

    /** Compile regular expression */
    reti = regcomp(&regex, gpio_pattern, REG_EXTENDED);
    if(reti)
    {
        return_code = CEREGCOMP;
        goto quit;
    }

    connect->type = GPIO;

    connect->inner_struct = (struct devdrvd_gpio_connect*)malloc(sizeof(struct devdrvd_gpio_connect));

    if(connect->inner_struct == 0) {
        perror("malloc");
        return_code = CEMALLOC;
        goto quit;
    }

    struct devdrvd_gpio_connect* current = (struct devdrvd_gpio_connect*)connect->inner_struct;

    current->in = 0;
    current->out = 0;


    struct cfg_t * device = cfg_getsec(cfg, "device");

    if(device) {        
        /** validate and init gpio */
        char* gpio = 0;
        int gpion[2];

        gpio = cfg_getstr(device, "gpio-in");

        /* Execute regular expression */
        reti = regexec(&regex, gpio, 0, NULL, 0);

        if( !reti )
        {
            char *pEnd;
            gpion[0] = strtol(gpio, &pEnd, 10);
            gpion[1] = strtol(++pEnd, 0, 10);


            if(gpion[0] > gpion[1]) {
                return_code = CEGPIORANGE;
                goto quit;
            }

            int j;
            for(j = gpion[0]; j <= gpion[1]; ++j)
                SET_BIT(current->in, j);

        }

        gpio = cfg_getstr(device, "gpio-out");

        /* Execute regular expression */
        reti = regexec(&regex, gpio, 0, NULL, 0);

        if( !reti )
        {
            char *pEnd;
            gpion[0] = strtol(gpio, &pEnd, 10);
            gpion[1] = strtol(++pEnd, 0, 10);


            if(gpion[0] > gpion[1]) {
                return_code = CEGPIORANGE;
                goto quit;
            }

            int j;
            for(j = gpion[0]; j <= gpion[1]; ++j)
                SET_BIT(current->out, j);

        }

    } else {
        return_code = CENOGPIODEVICE;
        goto quit;
    }

quit:
    regfree(&regex);
    return return_code;
}

