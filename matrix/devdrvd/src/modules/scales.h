#ifndef SCALES_H
#define SCALES_H

enum CONNECT_TYPE {
    COM_CONNECT,
    TCP_CONNECT
};

//device {
//    device="/dev/ttyAM1"
//    baudrate=115200
//    parity=none
//    stop-bits=1
//    data-bits=8
//}
struct devdrvd_serial_connect {
    char* device;
    unsigned int baudrate;
    unsigned int parity;
    unsigned int stop_bits;
    unsigned int data_bits;
};

//address {
//    ip=192.168.172.112
//    port=4001
//}
struct devdrvd_tcp_connect {
    char* ip;
    unsigned int port;
};

struct devdrvd_scale_connect {
    enum CONNECT_TYPE type;
    struct devdrvd_serial_connect* serial;
    struct devdrvd_tcp_connect* tcp;
};

//typedef int (*read_config_function)(struct cfg_t*, struct devdrvd_connect*);

struct devdrvd_connect;
struct cfg_t;

int read_scale_config(struct cfg_t*, struct devdrvd_connect*);

#endif // SCALES_H
