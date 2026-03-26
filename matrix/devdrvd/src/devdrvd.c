// SYSTEM INCLUDES
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <err.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <errno.h>

#include <pthread.h>

#include <strings.h>
#include <stdarg.h>

#include <regex.h>

#include <sys/time.h>

#include <dirent.h>
#include <stdio.h>

// PROJECT INCLUDES
#include "devdrvd.h"

#include "become_daemon.h"
#include "control_daemon.h"
#include "packet_daemon.h"
#include "command.h"
#include "connect.h"

#include "modules/scales.h"
#include "modules/gpios.h"

#include "devdrvd_communication.h"

#include "read_config.h"
#include <confuse.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


static int verbose_flag;
static int daemon_flag;
static int emulation_flag = 0;
int fast_flag = 0;

volatile sig_atomic_t shutdown_flag;
volatile sig_atomic_t reload_flag;
int rval;

unsigned int reconnect_interval=5;

pthread_mutex_t client_lock;
pthread_mutex_t connect_lock;

pthread_cond_t  clients_available;
volatile sig_atomic_t at_least_one;

struct devdrvd_client client_pool[DEVDRVD_MAX_CLIENTS];
unsigned int clients_number = 0;

static int control_sockfd = -1;  // server listen socket
static int devdrvd_ports = 0;

/** Devdrvd  */
struct devdrvd_connect **connect_array = 0;
int connect_array_size = 0;

struct devdrv_module **modules_array = 0;
int modules_array_size = 0;

//extern unsigned int reconnect_interval; // 30 seconds by default

extern int static_emulation_loop( char* logDir, char* logFile);

enum LISTEN_TYPE {
    IP_ADDRESS = 0,
    DEVICE_NAME
};

const char *listen_pattern[] = {
    "^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\:[0-9]{1,5}$",
    "^[a-zA-Z0-9\\.\\:]*$"
};

static struct option long_options[] =
{    
{"brief",          no_argument,        &verbose_flag,      0},
{"no-daemon",      no_argument,        &daemon_flag,       0},
{"emulate",        no_argument,        &emulation_flag,    1},
{"log-file",       required_argument,  0,                  'l'},
{"log-dir",        required_argument,  0,                  'D'},
{"version",        no_argument,        0,                  'v'},
{"config-file",    required_argument,  0,                  'e'},
{"ip-adress",      required_argument,  0,                  'i'},
{"port-number",    required_argument,  0,                  'p'},
{"help",           no_argument,        0,                  'h'},
{"static",         no_argument,        &emulation_flag,    2},
{0, 0, 0, 0}
};


/**************************************************************************
    Function: register_module

    Description:

    Params:

    Returns:

**************************************************************************/
void register_module(const char *name,
                     read_config_function func)
{
    modules_array_size++;
    modules_array = (struct devdrv_module**)realloc (modules_array, modules_array_size*sizeof(struct devdrv_module*));
    if(modules_array == 0) {
        perror("realloc");
        abort();
    }

    modules_array[modules_array_size - 1] = (struct devdrv_module*)malloc(sizeof(struct devdrv_module));

    struct devdrv_module* current = modules_array[modules_array_size - 1];

    int len = strlen(name);
    current->name = (char*)malloc(len + 1);
    strncpy(current->name, name, len + 1);

    current->read_config = func;
}

/**************************************************************************
    Function: free_modules

    Description:

    Params:

    Returns:

**************************************************************************/
void free_modules()
{
    int i;
    for(i = 0; i < modules_array_size; ++i)
    {
        free(modules_array[i]->name);
        free(modules_array[i]);
    }

    free(modules_array);
}


/**************************************************************************
    Function: _shutdown

    Description:
        Marks daemon for shutdown and shutdowns sockets

    Params:

    Returns:
        returns void always

**************************************************************************/
static void _shutdown()
{    
    shutdown_flag = 1;
}

/**************************************************************************
    Function: _reload

    Description:
        Marks daemon for config files reload

    Params:

    Returns:
        returns void always

**************************************************************************/
static void _reload()
{
    reload_flag = 1;
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

//{"emulate",        no_argument,        &emulation_flag,    1},
//{"log-file",       required_argument,  0,                  'l'},
//{"log-dir",        required_argument,  0,                  'D'},
void PrintUsage(int argc, char *argv[]) {
    if(argc >=1) {
        printf(
                    "--config-file         daemon configuration files directory\n" \
                    "-n                    Don't fork off as a daemon\n" \
                    "--ip-adress, -i       Set ip address for usage\n" \
                    "--port-number, -p     Set port number for usage\n" \
                    "--emulate             Read data from provided log file, don't connect to daemons\n" \
                    "--log-file, -l        Log file for reading\n" \
                    "--log-dir, -D         Log file directory for reading\n" \
                    "-h, --help            prints this message\n");
    }
}

/**
 * @brief unregister_client
 * @param client_pid
 * @note move to another place
 */
int unregister_client(int id)
{
    pthread_mutex_lock(&client_pool[id].lock);

    close(client_pool[id].write_fd);
    close(client_pool[id].read_fd);
    close(client_pool[id].socket_fd);

    client_pool[id].write_fd = -1;
    client_pool[id].read_fd = -1;
    client_pool[id].socket_fd = -1;

    pthread_mutex_lock(&client_lock);
    clients_number--;
    pthread_mutex_unlock(&client_lock);


    pthread_mutex_unlock(&client_pool[id].lock);

    return 0;
}

/**************************************************************************
    Function: signal_handler

    Description:
        This function handles select signals that the daemon may
        receive.  This gives the daemon a chance to properly shut
        down in emergency situations.  This function is installed
        as a signal handler in the 'main()' function.

    Params:
        @sig - The signal received

    Returns:
        returns void always
**************************************************************************/
void signal_handler(int sig) {

    switch(sig) {
    case SIGINT:
    case SIGTERM:
        _shutdown();
        //syslog(LOG_WARNING, "Received SIGTERM signal.");
        //syslog(LOG_INFO, "Cleaning and shutting down.");
        break;
    case SIGHUP:
        _reload();
        syslog(LOG_INFO, "Received SIGHUP signal.");
        syslog(LOG_INFO, "Reloading config file.");
        break;
    default:
        syslog(LOG_WARNING, "Unhandled signal (%d)", sig);
        break;
    }
}


 struct timeval get_time()
{
    struct timeval now;
    int rc;

    rc = gettimeofday(&now, NULL);
    if(rc==0) {
        return now;
    } else {
        abort();
    }
}

//#define MAX(i, j) (((i) > (j)) ? (i) : (j))

static int	iStartTrain=0;
static void handlerSigUsr1(int sig)
{
	iStartTrain=1;
}
void emulation_loop(char* logDir, char* logFile)
{
    DIR *dpdf = NULL;
    struct dirent *epdf;
    int rewind = 0;

    char* package = 0;
    size_t package_len = 0;


    struct weight_struct w_s;
    struct discrete_struct d_s;

	int cnt_channels = 0;

	unsigned long timestamp_w=0;
	
    printf("emulation_loop(0x%p; 0x%p) called.\n", logDir, logFile);

    if(logDir != 0) {
        dpdf = opendir(logDir);
    }

//<-nik
// Обработчик сигнала на SIGUSR1
	iStartTrain = 0;
	if( signal(SIGUSR1, handlerSigUsr1) == SIG_ERR) {
		printf( "emulation_loop: error signal( SIGUSR1) %d(%s)\n", errno, strerror( errno));
		exit( EXIT_FAILURE);
	}
	
    while(shutdown_flag != 1) {
        struct timeval tv;
        int weightPackets = 0;

	printf("emulation_loop[%05d] Dummy loop...\n", __LINE__);

	
        while (shutdown_flag != 1) {
        	
            tv.tv_sec = 0;
            tv.tv_usec = 60000;

            select(0, 0, 0, 0, &tv);
			
			if( timestamp_w != 0)
				timestamp_w += 10;
				 	
			for( int _i = 0; _i <= cnt_channels; ++_i) { 
//if( fast_flag && timestamp_w) {
//				struct timeval tv;
       			tv.tv_sec = timestamp_w/1000;
       			tv.tv_usec = (timestamp_w%1000) * 1000.0;
                w_s.ts = tv;

//}
//else				
//            	w_s.ts = get_time();
            	
            	w_s.channel = _i;
            	w_s.status = 0;
            	w_s.tare = 0.0;
            	w_s.weight = 0.0;

            	package_len = pack_weight(&package, &w_s);

            	/** Dispach data to registered clients */
            	int i = 0;
            	for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i) {
                	if(client_pool[i].socket_fd != -1) {
                		pthread_mutex_lock(&client_pool[i].lock);
                	    write(client_pool[i].write_fd, package, package_len);
                		pthread_mutex_unlock(&client_pool[i].lock);
                	  }
            	}
			}
            free(package);

            weightPackets++;

            if(weightPackets >= 50)
                break;
        }
        
       	if( !iStartTrain) {
       		continue;
       	}


        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

	printf("devdrvd[%05d]::emulation_loop() Starting file emulation...\n", __LINE__);
	printf("               dpdf = 0x%p; logFile = 0x%p.\n", dpdf, logFile);

        /** */
        if(dpdf != 0) {
            epdf = readdir(dpdf);
            if(epdf == 0) {
                rewinddir(dpdf);
                ++rewind;
                epdf = readdir(dpdf);
                if(epdf == 0) {
                    /** no files in dir*/
                    printf("No files in directory: %s\n", logDir);
                    break;
                }
            }

            if(epdf->d_type != DT_REG)
            {
                printf("%s is not a regular file. Skipping...\n", epdf->d_name);
                continue;
            }
            
            if( !strstr( epdf->d_name, ".log")) {
                printf("%s is not a log file. Skipping...\n", epdf->d_name);
                continue;
            }

            char fileName[1204];
            sprintf(fileName, "%s%s", logDir, epdf->d_name);
            fp = fopen(fileName, "r");
            printf("Reading data from file: %s\n", fileName);
        } else {
            fp = fopen(logFile, "r");
            printf("Reading data from file: %s\n", logFile);
        }

        if (fp == 0)
        {
            printf("Error opening file.\n");
            exit(EXIT_FAILURE);
        }

	printf("devdrvd[%05d]::emulation_loop() File to be emulated opened.\n", __LINE__);

        unsigned long timestamp = 0;
        unsigned long prev_timestamp = 0;
        unsigned int delta = 0;
//
	unsigned int delta_send = 0;
        struct timeval tv_start, tv_end;
//		struct timeval tv;
	        
        //   [ts]  : [packet_type] : data
        //   63928 : W0 :          38420
        //   64281 : DT :               2   DN                  ( 30)
        //   64113 : DT :               2   UP                  ( 29)

        while((read = getline(&line, &len, fp)) != -1) {

            if(shutdown_flag != 0)
            {
               printf("Emulation loop: shutdown_flag\n");
               goto cleanup;
            }

            if(line[0] == '!' || line[0] == '*')
                continue;

            char * pNext = 0;
            int w_id = -1;
            int wd_id = -1;
            char wd_lh = -1; // low/high

            timestamp = strtol(line, &pNext, 10); // in nanoseconds

            pNext = strstr(pNext, ":");

            if(pNext == 0)
                continue;

            pNext += 2;
			if( *pNext != 'W' && *pNext != 'D')
				continue;

if( !fast_flag) {
            //if(timestamp - prev_timestamp > 0) {
            delta = timestamp - prev_timestamp;
            delta *= 1000000;
            prev_timestamp = timestamp;

            struct timespec tv;
            tv.tv_sec = 0;
//            tv.tv_nsec = delta;
		if( delta > delta_send) {
            		tv.tv_nsec = delta - delta_send;
            		nanosleep(&tv, 0);
		}
}		
/*
            pNext = strstr(pNext, ":");

            if(pNext == 0)
                continue;

            pNext += 2;
*/
//            nanosleep(&tv, 0);

/*
else {
        struct timespec tv;
        tv.tv_sec = 0;
        tv.tv_nsec = 1000000;
   		nanosleep(&tv, 0);
}
*/
		gettimeofday( &tv_start, NULL);
            switch(*pNext) {
            case 'W':
                printf("Timestamp : %lu : ", timestamp);
                w_id = strtol(++pNext, 0, 10);
                cnt_channels = MAX( cnt_channels, w_id);

                pNext = strstr(pNext, ":");
                float weight = strtof(++pNext, 0);//  strtol(++pNext, 0, 10);
                printf("Weight from platform #%d : %f\n", w_id, weight);

if( fast_flag) {
//				struct timeval tv;
				timestamp_w = timestamp; 
       			tv.tv_sec = timestamp/1000;
       			tv.tv_usec = (timestamp%1000) * 1000.0;
                w_s.ts = tv;
}                
else  {
//                w_s.ts = get_time();
				timestamp_w = timestamp; 
       			tv.tv_sec = timestamp/1000;
       			tv.tv_usec = (timestamp%1000) * 1000.0;
                w_s.ts = tv;

}                
                w_s.channel = w_id;
                w_s.status = 0;
                w_s.tare = weight;
                w_s.weight = weight;

                package_len = pack_weight(&package, &w_s);

                printf("Packed weight: %s\n", package);


                /** Dispach data to registered clients */
                int i = 0;
                for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i) {
                    pthread_mutex_lock(&client_pool[i].lock);
                    if(client_pool[i].socket_fd != -1)
                        write(client_pool[i].write_fd, package, package_len);
                    pthread_mutex_unlock(&client_pool[i].lock);
                }

                free(package);
                break;
            case 'D':
                printf("Timestamp : %lu : ", timestamp);
                pNext = strstr(pNext, ":");
                wd_id = strtol(++pNext, &pNext, 10);
                while(isspace(*(++pNext)));
                switch (*pNext)
                {
                case 'U':
                    wd_lh = 1;
                    break;
                case 'D':
                    wd_lh = 0;
                    break;
                default:
                    break;
                }

                char buffer[10];
                printf("Wheel detector #%d : %d\n", wd_id, wd_lh);

if( fast_flag)  {
//				struct timeval tv;
       			tv.tv_sec = timestamp/1000;
       			tv.tv_usec = (timestamp%1000) * 1000.0;
                d_s.ts = tv;
}
else
                d_s.ts = get_time();
                
                d_s.channel = wd_id;
                d_s.value = wd_lh;

                package_len = pack_discrete(&package, &d_s);
 
                printf("Packed discrete: %s\n", package);

                for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i) {
                    pthread_mutex_lock(&client_pool[i].lock);
                    if(client_pool[i].socket_fd != -1)
                        write(client_pool[i].write_fd, package, package_len);
                    pthread_mutex_unlock(&client_pool[i].lock);
                }

              
                free(package);
                break;
            default:
            	break;
//                continue;
            }

		gettimeofday( &tv_end, NULL);
		delta_send = (tv_end.tv_sec - tv_start.tv_sec)*1000 + (tv_end.tv_usec - tv_start.tv_usec)/1000000.0;
            
        }

        cleanup:
        if (line)
            free(line);

        fclose(fp);
        iStartTrain = 0;
    } // while
}

/**************************************************************************
    Function: open_and_check

    Description:
        This function test if file exists and is readable
    Params:
        @fileName - full filepath
    Returns:
        returns: 0 - Ok, 1 - Error
**************************************************************************/
int open_and_check(const char* fileName)
{
    if( access(fileName, R_OK ) != -1 )
        return 0;

    perror(fileName);
    return 1;
}

int main(int argc, char *argv[], char* envp[])
{
    shutdown_flag = 0;
    reload_flag = 0;

    /** Sockets configuration parameters */
    char *ipaddr = 0; // ip adress for open
    //char *device = 0;
    //char *username = 0;
    //char *group = 0;
    char *configFile = 0;

    char *logFile = 0;
    char *logDir = 0;
	char *staticConf = NULL;
	
    int listen_portno = 1500;
	reconnect_interval = 5;
	
    int c;
    int option_index = 0;
    while((c = getopt_long(argc, argv, "e:hi:p:vf", long_options, &option_index)) != -1) {
        switch(c){
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;
        case 'v' :
        {
            printf("%s\n", "0.0.0");
            exit(EXIT_SUCCESS);
            break;
        }
        case 'e' : {
            if(open_and_check(optarg) != 0) {
                /** @todo print_and_exit function
                 */
                printf("Error opening file --config-file=%s", optarg);
                exit(EXIT_FAILURE);
            }
            configFile = optarg;
            break;
        }
        case 'i':
            ipaddr = optarg;
            printf("Ipadress given: %s\n", ipaddr);
            break;
        case 'p':
            listen_portno = atoi(optarg);
            if(listen_portno == 0) {
                perror("Please give me corrent port number.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l' :
            logFile = optarg;
            if(access(logFile, R_OK) != 0) {
                printf("Error accessing the given log file: %s\n", logFile);
                perror(logFile);
                exit(EXIT_FAILURE);
            }
            printf("Taking data from: %s\n", logFile);
            break;
        case 'D' :
            logDir = optarg;
            if(access(logDir, R_OK | X_OK) != 0) {
                printf("Error accessing the given directory: %s\n", logDir);
                perror(logDir);
                exit(EXIT_FAILURE);
            }
            printf("Taking data from: %s\n", logDir);
            break;
        case 'h':
            PrintUsage(argc, argv);
            return EXIT_SUCCESS;
            break;
        case 'f':
        	fast_flag = 1;
        	break;
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            break;
        }
    }

    if( emulation_flag && logFile == 0 && logDir == 0 ) {
        printf("Please provide log filename or directory with --log-file or --log-dir option.\n");
        exit(EXIT_FAILURE);
    }

    printf("devdrvd[%05d]::main() Options parsed.\n", __LINE__);

    openlog(DAEMON_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);

    /** Initialize client pool */
    clients_number = 0;
    volatile int i;
    for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i) {
        client_pool[i].write_fd = -1;
        client_pool[i].read_fd = -1;
        client_pool[i].socket_fd = -1;

        pthread_mutex_init(&client_pool[i].lock, 0);
    }

    printf("devdrvd[%05d]::main() Clients pool created.\n", __LINE__);

    /** Initializing section for read config
      */
    register_module("scale", read_scale_config);
    register_module("gpio", read_gpio_config);

    if(configFile == 0)
    {
        printf("Please provide config filename with --config-file option.\n");
        exit(EXIT_FAILURE);
    }

    /** @begin Config file read */
    if(read_config(configFile) != CFG_SUCCESS)
    {
        printf("fatal: read_config");
        exit(EXIT_FAILURE);
    }

    get_sections(&connect_array, &connect_array_size);
    /** @end */

    /** @begin Get ip address and port for listen */
    get_ipadress(&ipaddr);
    get_port(&listen_portno);
    /** @end*/

    printf("devdrvd[%05d]::main() Configuration processed.\n", __LINE__);

#ifdef DEBUG
    printf("Ip address : %s : %i\n", ipaddr, listen_portno);
#endif
    /** Init connect and client mutexes*/
    //    pthread_mutex_t client_lock;
    //    pthread_mutex_t connect_lock;

    pthread_mutex_init(&client_lock, NULL);
    
    pthread_mutex_init(&connect_lock, NULL);
    pthread_cond_init(&clients_available, NULL);
    at_least_one = 0;

    printf("devdrvd[%05d]::main() Mutexes and Condvars are initialized.\n", __LINE__);

    /** @begin Launch connect threads */
/* 
 * Создание нитей клиентских соединений к серверам - поставщикам данных 
 */    
    if(emulation_flag == 0)
        for(i = 0; i < connect_array_size; i++) {
            if(pthread_create(&connect_array[i]->connect_thread, NULL, connect_client, (void*)connect_array[i])) {
                perror("Could not create connect thread");
                syslog(LOG_CRIT, "Could not create connect thread");
                return EXIT_FAILURE;
            }
        }
    /** @end */

    /** Now start listening for the clients, here
     *  process will go in sleep mode and will wait
     *  for the incoming connection
     */

    printf("devdrvd[%05d]::main() connect thread started.\n", __LINE__);

    int maxfd = 0;
    int ready;
    fd_set readfds;
    fd_set errfs;

    /** Mask Signals and enstablish handlers
      */
    struct sigaction sa;
    sigset_t prevmask, blockset;

    sigemptyset(&blockset);         /* Block SIGINT */
    sigaddset(&blockset, SIGINT);
    sigaddset(&blockset, SIGHUP);
    sigaddset(&blockset, SIGTERM);
    sigaddset(&blockset, SIGQUIT);
    sigaddset(&blockset, SIGCHLD);

    sigprocmask(SIG_BLOCK, &blockset, &prevmask);

    sa.sa_handler = signal_handler;        /* Establish signal handler */
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);

    sigemptyset(&prevmask);

    printf("devdrvd[%05d]::main() Signal Actions activated.\n", __LINE__);

    /** Initializing server socket
     */
    syslog(LOG_INFO, "Listening on: %s:%d\n", ipaddr, listen_portno);
    int result = init_listen_socket(ipaddr, listen_portno, &control_sockfd);

    printf("devdrvd[%05d]::main() Listen socket started [result=%d].\n", __LINE__, result );

    if(result != 0) {
        printf("%s\n", strerror(result));
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Server socket: %d\n", control_sockfd);

    pthread_t control;
    if(pthread_create(&control, NULL, runcontrol, (void*)&control_sockfd) != 0){
        perror("Could not create server thread");
        syslog(LOG_CRIT, "Could not create server thread");
        return EXIT_FAILURE;
    }

    printf("devdrvd[%05d]::main() runcontrol thread started.\n", __LINE__);

    /** Unblock signal, then wait for signal or ready file descriptor */
    sigprocmask(SIG_SETMASK, &prevmask, 0);

    /** emulation */
    if(emulation_flag == 1) {
	printf("Emulation flag: %d; Starting Emulation loop...\n", emulation_flag);
        emulation_loop(logDir, logFile);
        goto shutdown;
    }

	else if( emulation_flag == 2) {
        static_emulation_loop(logDir, logFile);
        goto shutdown;
	}
    while(!shutdown_flag) {
        /** Setting  descriptors for select */
        FD_ZERO(&readfds);
        FD_ZERO(&errfs);

        pthread_mutex_lock(&connect_lock);

        for(i = 0; i < connect_array_size; ++i)
            if(connect_array[i]->connected == 1) {
                maxfd = (maxfd < connect_array[i]->sockfd) ? connect_array[i]->sockfd : maxfd;
                FD_SET(connect_array[i]->sockfd, &readfds);
                FD_SET(connect_array[i]->sockfd ,&errfs);
            }

        pthread_mutex_unlock(&connect_lock);

        if(maxfd == 0)
            continue;

        /** timeout for each device */
        struct timespec tv;
        tv.tv_sec = 3;
        tv.tv_nsec = 0;

        ready = pselect(maxfd + 1, &readfds, NULL, &errfs, &tv, &prevmask);

        int errsv = errno;
        sigprocmask(SIG_SETMASK, &prevmask, 0);

        pthread_mutex_lock(&connect_lock);

        if (ready == -1) {
#ifdef DEBUG
            printf("pselect() ready : %d ; %d ;%s\n", ready, errsv, strerror(errsv));
            syslog(LOG_DEBUG, "Shutdown_flag=%d, errno=%d", shutdown_flag, errno);
#endif           
            if(errsv == EINTR && shutdown_flag)
            {
                syslog(LOG_INFO, "Shutdown called");
                shutdown(control_sockfd, SHUT_RD);
                pthread_kill(control, SIGINT);
                continue;
            }

            if(errsv == EBADF)
            {
                for(i = 0; i < connect_array_size; ++i)
                    if(connect_array[i]->connected == 1)
                    {
                        if(fcntl(connect_array[i]->sockfd, F_GETFL) < 0) {
                            connect_array[i]->connected = 0;
                            close(connect_array[i]->sockfd);
                            connect_array[i]->sockfd = -1;

                            if(pthread_create(&connect_array[i]->connect_thread, NULL, connect_client, (void*)connect_array[i]) != 0)
                            {
                                perror("Could not create connect thread");
                                syslog(LOG_CRIT, "Could not create connect thread");
                                return EXIT_FAILURE;
                            }
                        }
                    }
                pthread_mutex_unlock(&connect_lock);
                continue;
            }
            syslog(LOG_CRIT, "select on channels returned error: %s", strerror(errsv));
        } else if(ready) {
            for(i = 0; ready > 0, i < connect_array_size; ++i)
                if(connect_array[i]->sockfd > 0 && FD_ISSET(connect_array[i]->sockfd, &readfds))
                {
                    int len = 0;
                    ioctl(connect_array[i]->sockfd, FIONREAD, &len);
                    errsv = errno;

                    /** @begin Comment for dummy pipe */
                    if(len == 0)// && errsv == EADDRNOTAVAIL)
                    {
                        /** Connection to daemon crashed
                         * @todo send error code to client
                         */
                        connect_array[i]->connected = 0;
#ifdef DEBUG
                        printf("Server %s:%d possibly crashed trying to reconnect...\n", connect_array[i]->address, connect_array[i]->port);
#endif
                        syslog(LOG_ERR, "Server %s:%d possibly crashed trying to reconnect...\n", connect_array[i]->address, connect_array[i]->port);
                        //shutdown(connect_array[i]->sockfd, SHUT_RD);
                        close(connect_array[i]->sockfd);
                        connect_array[i]->sockfd = -1;

                        if(pthread_create(&connect_array[i]->connect_thread, NULL, connect_client, (void*)connect_array[i]) != 0)
                        {
                            perror("Could not create connect thread");
                            syslog(LOG_CRIT, "Could not create connect thread");
                            return EXIT_FAILURE;
                        }
                    }
                    /** @end Comment for dummy pipe */

                    unsigned int readen;
                    while(len > 0) {
                        char data[1024] = {0};

                        if(len > sizeof(data)) {
                            readen = read(connect_array[i]->sockfd, &data, sizeof(data));
                        } else {
                            readen = read(connect_array[i]->sockfd, &data, len);
                        }

                        if(readen == -1)
                        {
#ifdef DEBUG
                            printf("Server %s:%d possibly crashed trying to reconnect...\n", connect_array[i]->address, connect_array[i]->port);
#endif
                            connect_array[i]->connected = 0;
                            close(connect_array[i]->sockfd);
                            connect_array[i]->sockfd = -1;

                            if(pthread_create(&connect_array[i]->connect_thread, NULL, connect_client, (void*)connect_array[i]) != 0)
                            {
                                perror("Could not create connect thread");
                                syslog(LOG_CRIT, "Could not create connect thread");
                                return EXIT_FAILURE;
                            }

                            break;
                        }
                        printf("Read data (%d): %s\n", readen, data);

                        /** Dispach data to registered clients */
                        int i = 0;
                        ssize_t written;
                        for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i)
                        {
                            pthread_mutex_lock(&client_pool[i].lock);
                            if(client_pool[i].socket_fd != -1)
                            {
#ifdef VERBOSE
                                printf("Writing to client pid: %d\t pipe_fd: %d data: %s\n", client_pool[i].client_pid, client_pool[i].write_fd, data);
#endif
                                /**
                                 * @todo check errors
                                 */
                                printf("Writing to client pipe_fd: %d data: %s\n", client_pool[i].write_fd, data);
                                
                                written = write(client_pool[i].write_fd, &data, readen);

                                if(written == -1) {
                                    int errsv = errno;                                   
                                    syslog(LOG_CRIT, "Failed writing data to client: %s [%d]", strerror(errsv), errsv);
                                    /** disconnect here ? */                                    
                                }
                            }
                            pthread_mutex_unlock(&client_pool[i].lock);
                        }

                        len -= readen;
#ifdef VERBOSE
                        if(len != 0)
                            printf("Data left %d bytes: %s\n", len, data);
#endif
                    }
                    ready--;
                }
        }
        pthread_mutex_unlock(&connect_lock);
    }
    /** Unregister clients */

shutdown:
    close(control_sockfd);
    pthread_cancel(control);
    pthread_join(control, NULL);
    syslog(LOG_INFO, "Exiting...");
    free_modules();
    free_config();
    closelog();
    return EXIT_SUCCESS;
}
