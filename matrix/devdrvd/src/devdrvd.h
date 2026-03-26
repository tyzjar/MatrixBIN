#ifndef DEVDRVD_H
#define DEVDRVD_H

#ifndef DAEMON_NAME
#define DAEMON_NAME "devdrvd" // default daemon name
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE CONFDIR "/" DAEMON_NAME ".conf" // default daemon conf file
#endif

#ifndef DEVDRVD_MAX_CLIENTS // i am very lazy
#define DEVDRVD_MAX_CLIENTS 10
#endif

#include <pthread.h> // temporary

#define MAX(i, j) (((i) > (j)) ? (i) : (j))

extern unsigned int reconnect_interval;

struct devdrvd_client {
    int read_fd;
    int write_fd;
    int socket_fd;

    pthread_t control;
    pthread_mutex_t lock;
};

struct devdvd_init {
    const char* listen_on;  //listen-on={localhost}
    unsigned int port;      //port=6000
    unsigned int cport;     //cport=6001
};

enum DEVDRVD_TYPE
{
    GPIO = 0,
    SCALE_TCP,
    SCALE_SERIAL
};

typedef int (*init_function)(int);

struct cfg_t;
struct devdrvd_connect;
typedef int (*read_config_function)(struct cfg_t*, struct devdrvd_connect*);

typedef int (*parse_data)(const char*);
typedef char* (*pack_data)();

//struct pthread_t;

struct devdrvd_connect {
    enum DEVDRVD_TYPE type;

    pthread_t connect_thread;

    char id; // ?
    char connected;
    int errsv;

    int sockfd;
    int control_sockfd;

    char* address;

    unsigned int port; // separate control/data port
    unsigned int cport;

    char* buffer;
    unsigned int buffer_length;

    char* name;

    init_function init;
    read_config_function read_config;

    void* inner_struct;
};

struct devdrv_module {
    char* name;
    read_config_function read_config;
};

void register_module(const char*, read_config_function);
int unregister_client(int);

#endif // GPIODAEMON_H
