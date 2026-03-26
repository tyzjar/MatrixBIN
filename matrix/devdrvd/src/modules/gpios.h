#ifndef GPIOS_H
#define GPIOS_H

#ifndef SET_BIT
#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
#endif

//gpio 0 {
//    ip="localhost"
//    port=7000
//    cport=7001
//    device {
//        gpio-in=0:7
//        gpio-out=8:15
//    }
//}

struct devdrvd_gpio_connect {
    int in;
    int out;
};

struct cfg_t;
struct devdrvd_connect;

int read_gpio_config(struct cfg_t*, struct devdrvd_connect*);

#endif // GPIOS_H
