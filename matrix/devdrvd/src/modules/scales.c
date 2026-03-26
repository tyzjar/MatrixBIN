#include "scales.h"

#include "../devdrvd.h"
#include "../read_config.h"

#include <confuse.h>
#include <stdlib.h>

int read_scale_config(struct cfg_t * cfg, struct devdrvd_connect * connect)
{
    struct cfg_t * address = 0;
    struct cfg_t * device = 0;

    address = cfg_getsec(cfg, "address");
    device = cfg_getsec(cfg, "device");

    if(address) {
        connect->type = SCALE_TCP;

        connect->inner_struct = malloc(sizeof(struct devdrvd_tcp_connect));
        struct devdrvd_tcp_connect* current = (struct devdrvd_tcp_connect*)connect->inner_struct;

        //            address {
        //                ip=192.168.172.112
        //                port=4001
        //            }

        current->ip = cfg_getstr(address, "ip");
        current->port = (unsigned int)cfg_getint(address, "port");
    } else if (device) {        
        connect->type = SCALE_SERIAL;

        connect->inner_struct = (struct devdrvd_serial_connect*)malloc(sizeof(struct devdrvd_serial_connect));        
        struct devdrvd_serial_connect* current = (struct devdrvd_serial_connect*)connect->inner_struct;

        //            device {
        //                device="/dev/ttyAM1"
        //                baudrate=115200
        //                parity=none
        //                stop-bits=1
        //                data-bits=8
        //            }

        current->device = cfg_getstr(device, "device");
        current->baudrate = (unsigned int)cfg_getint(device, "baudrate");
        /**
         * @todo parity
         */
        current->parity = 0;

        current->stop_bits = (unsigned int)cfg_getint(device, "stop-bits");
        current->data_bits = (unsigned int)cfg_getint(device, "data-bits");
    } else {
        return CENOSCALETYPE;
    }

    return CESUCCESS;
}
