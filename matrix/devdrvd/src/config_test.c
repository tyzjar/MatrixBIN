/* Packing Test Program */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>

#include <regex.h>

#include <confuse.h>

#include "devdrvd.h"
#include "read_config.h"

#include "modules/scales.h"
#include "modules/gpios.h"


static struct option long_options[] =
{
    {"config-file",     required_argument,  0,                  'e'},
    {"help",            no_argument,        0,                  'h'},
    {0, 0, 0, 0}
};

extern const char *gpio_pattern;// =  "^[0-9]{1,3}\\:[0-9]{1,3}$";

struct devdrvd_connect **connect_array = 0;
int connect_array_size = 0;

struct devdrv_module **modules_array = 0;
int modules_array_size = 0;

void register_module(const char *name,
                     read_config_function func)
{
    modules_array_size++;
    modules_array = (struct devdrv_module**)realloc (modules_array, modules_array_size*sizeof(struct devdrv_module*));
    modules_array[modules_array_size - 1] = (struct devdrv_module*)malloc(sizeof(struct devdrv_module));

    struct devdrv_module* current = modules_array[modules_array_size - 1];

    int len = strlen(name);
    current->name = (char*)malloc(len);
    strncpy(current->name, name, len);

    current->read_config = func;
}

int validate_unsigned_gpio(cfg_t *cfg, cfg_opt_t *opt)
{
    regex_t regex;
    int reti;

    /** Compile regular expression */
    reti = regcomp(&regex, gpio_pattern, REG_EXTENDED);

    if(reti)
    {
        cfg_error(cfg, "Coudn't compile regular expression for %s in %s", opt->name, cfg->name);
        return -1;
    }

    char* gpio = cfg_opt_getnstr(opt, cfg_opt_size(opt) - 1);

    int gpion[2];

    /* Execute regular expression */
    reti = regexec(&regex, gpio, 0, NULL, 0);

    if( !reti ) {
        char *pEnd;
        gpion[0] = strtol(gpio, &pEnd, 10);
        gpion[1] = strtol(++pEnd, 0, 10);


        if(gpion[0] > gpion[1]) {
            cfg_error(cfg, "First part of expression should be less or equal for %s in %s", opt->name, cfg->name);
            return -1;
        }

    } else {
        cfg_error(cfg, "Value for gpio doesn't match pattern. %s in %s", opt->name, cfg->name);
        return -1;
    }

    return 0;
}


int open_and_check(const char* fileName)
{
    if( access(fileName, R_OK ) != -1 )
        return 0;

    perror(fileName);
    return 1;
}

/**************************************************************************
    Function: Print Usage

    Description:
        Output the command-line options for this daemon.

    Params:
        @argc - Standard argument count
        @argv - Standard argument array

    Returns:
        returns void always

**************************************************************************/
void PrintUsage(int argc, char *argv[]) {
    if(argc >=1) {
        printf(
                    "--configfile         daemon configuration files directory\n" \
                    "-h, --help           prints this message\n");
    }
}


int main(int argc, char *argv[], char* envp[])
{
    char *configFile = 0;
    int c;

    int option_index = 0;
    while((c = getopt_long(argc, argv, "e:h", long_options, &option_index)) != -1) {
        switch(c) {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;
        case 'e' : {
            if(open_and_check(optarg) != 0) {
                /** @todo print_and_exit function
                 */
                printf("Error opening file --configfile=%s\n", optarg);
                exit(EXIT_FAILURE);
            }
            configFile = optarg;
            break;
        }
        case 'h':
            PrintUsage(argc, argv);
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            break;
        }
    }

    /** Registering gpio modules */
    register_module("scale", &read_scale_config);
    register_module("gpio", &read_gpio_config);

    if(configFile == 0)
    {
        printf("Please provide config filename with --config-file option.\n");
        exit(EXIT_FAILURE);
    }

    /** @begin config file test */
    if(read_config(configFile) != CFG_SUCCESS)
    {
        printf("fatal: read_config");
        exit(EXIT_FAILURE);
    }


    int _err = 0;

    get_sections(&connect_array, &connect_array_size);

    printf("Devdrvd connect array size=%d\n", connect_array_size);

    int i;
    for(i = 0; i < connect_array_size; ++i) {

        printf("Connect #%d\n", connect_array[i]->id);
        printf("\tip=%s\n", connect_array[i]->address);
        printf("\tport=%d\n", connect_array[i]->port);
        printf("\tcport=%d\n", connect_array[i]->cport);

        switch(connect_array[i]->type) {
        case SCALE_SERIAL:
        {
            struct devdrvd_serial_connect* current = (struct devdrvd_serial_connect*)(connect_array[i]->inner_struct);
            printf("\tSerial port parameters:\n");
            printf("\t\tdevice=%s\n", current->device);
            printf("\t\tbaudrate=%d\n", current->baudrate);
            printf("\t\tdata-bits=%d\n", current->data_bits);
            printf("\t\tstop-bits=%d\n", current->stop_bits);
            printf("\t\tparity=%d\n", current->parity);
        }
            break;
        case SCALE_TCP:
        {
            struct devdrvd_tcp_connect* current = (struct devdrvd_tcp_connect*)(connect_array[i]->inner_struct);
            printf("\tTcp socket parameters:\n");
            printf("\t\tip=%s\n", current->ip);
            printf("\t\tport=%d\n", current->port);
        }
            break;
        case GPIO :
        {
            struct devdrvd_gpio_connect* current = (struct devdrvd_tcp_connect*)(connect_array[i]->inner_struct);
            printf("\tGpio parameters:\n");
            printf("\t\tin=%d\n", current->in);
            printf("\t\tout=%d\n", current->out);
        }
            break;
        default:
            printf("Error: No type specified for connection: %d\n", connect_array[i]->id);
        }        
    }


    free_config();
    /** @end config file test */

    return EXIT_SUCCESS;
}
