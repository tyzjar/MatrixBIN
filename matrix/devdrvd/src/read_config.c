#include "read_config.h"

#include <stdlib.h>
#include <regex.h>

#include <confuse.h>

//#include <devdrvd_communication.h>
#include "devdrvd_communication.h"


#include "modules/gpios.h"
#include "modules/scales.h"

/** Human readable errors */
const char *config_errors[] = {
    "Success.",
    "No type given for scale connection.",
    "Gpio number not in range.",
    "No device specified for gpio section."
};

/** Config struct */
static cfg_t *cfg = 0;

static int errl = -1;

int errline()
{
    return errl;
}

int read_config(const char *fileName)
{
    static cfg_opt_t device_opts[] = {
        CFG_STR("device", 0, CFGF_NONE),
        CFG_INT("baudrate", 115200, CFGF_NONE),
        CFG_STR("parity", "none", CFGF_NONE),
        CFG_INT("stop-bits", 1, CFGF_NONE),
        CFG_INT("data-bits", 8, CFGF_NONE),
        CFG_END()
    };

    static cfg_opt_t address_opts[] = {
        CFG_STR("ip", "localhost", CFGF_NONE),
        CFG_INT("port", 4001, CFGF_NONE),
        CFG_END()
    };

    static cfg_opt_t scale_opts[] = {
        CFG_STR("ip", "localhost", CFGF_NONE),
        CFG_INT("port", 5000, CFGF_NONE),
        CFG_INT("cport", 5001, CFGF_NONE),
        CFG_SEC("device", device_opts, CFGF_MULTI),
        CFG_SEC("address", address_opts, CFGF_MULTI),
        CFG_END()
    };

    static cfg_opt_t gpio_device_opts[] = {
        CFG_STR("gpio-in", "0:7", CFGF_NONE),
        CFG_STR("gpio-out", "8:15", CFGF_NONE),
        CFG_END()
    };

    static cfg_opt_t gpio_opts[] = {
        CFG_STR("ip", "localhost", CFGF_NONE),
        CFG_INT("port", 5000, CFGF_NONE),
        CFG_INT("cport", 5001, CFGF_NONE),
        CFG_SEC("device", gpio_device_opts, CFGF_MULTI),
        CFG_END()
    };

    cfg_opt_t opts[] = {
        CFG_STR_LIST((char *)"listen-on",  (char *)"{localhost}", CFGF_NONE),
        CFG_INT((char *)"port", 6000, CFGF_NONE),
        CFG_INT((char *)"cport", 6001, CFGF_NONE),
        CFG_INT((char *)"polling", 10, CFGF_NONE),
        CFG_SEC("scale", scale_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("gpio", gpio_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_END()
    };

    cfg = cfg_init(opts, CFGF_NONE);

    return cfg_parse(cfg, fileName);
}

int get_ipadress(char **ip_address)
{
    *ip_address = cfg_getstr(cfg, "listen-on");
    return CESUCCESS;
}


int get_port(int *port)
{
    *port = cfg_getint(cfg, "port");
    return CESUCCESS;
}

int get_sections(struct devdrvd_connect*** section_array, int* size)
{
    extern struct devdrv_module **modules_array;
    extern int modules_array_size;
    (*size) = 0;

    int i;

    for(i = 0; i < modules_array_size; ++i) {
        int n = cfg_size(cfg, modules_array[i]->name); // config name

        if(n == 0)
            continue;

        *section_array = (struct devdrvd_connect***)realloc(*section_array, ((*size) + n)*sizeof(struct devdrvd_connect*));

        if(*section_array == 0) {
            perror("realloc");
            abort();
        }

        int j,k;
        int len = n + (*size);
        for(j = (*size), k = 0; j < len; ++j, ++k) {
            (*size)++;
            (*section_array)[j] = (struct devdrvd_connect*)malloc(sizeof(struct devdrvd_connect));
            struct devdrvd_connect* current = (*section_array)[j];

            cfg_t *bm = cfg_getnsec(cfg, modules_array[i]->name, k);

            current->id = j + 1;
            current->connected = 0;

            current->buffer = 0;
            current->name = 0;

            current->address = cfg_getstr(bm, "ip");
            current->port = (unsigned int)cfg_getint(bm, "port");
            current->cport = (unsigned int)cfg_getint(bm, "cport");

            current->inner_struct = 0;

            modules_array[i]->read_config(bm, current);
        }
    }

    return *size;
}

void free_sections(struct devdrvd_connect *** section_array, int size)
{

}


int free_config()
{
    cfg_free(cfg);
    cfg = 0;
}
