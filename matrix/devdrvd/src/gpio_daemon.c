#include "gpio_daemon.h"
#include "devdrvd.h"
#include "connect.h"

#include <syslog.h>
#include <error.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include <sys/select.h>
#include <sys/ioctl.h>

#include <errno.h>

#include "comm/devdrvd_communication.h"

//static struct devdrvd_scale_connect **scale_array = 0;
//static int scale_array_size = 0;
//static struct devdrvd_gpio_connect **gpio_array = 0;
//static int gpio_array_size = 0;

extern struct devdrvd_gpio_connect **gpio_array;
extern int gpio_array_size;
extern volatile sig_atomic_t shutdown_flag;

extern struct devdrvd_client client_pool[];

void connect_gpio(void *contex) /** connect is common for all daemons? */
{
//    int index = *((int*)contex);
//    int result = 0;

//    if(index >= gpio_array_size) // fatal
//        pthread_exit((void*)&result);

//    struct devdrvd_gpio_connect* current = gpio_array[index];

//    if(current == 0) //fatal
//        pthread_exit((void*)&result);


//    while(shutdown_flag != 1 || current->connected != 1) {

//        /** array element can be wiped out so check */
//        result = init_socket(current->address, current->port, &current->sockfd);

//        switch(result) {
//        /** nothing can be done: */
//        case EACCES : //The address is protected, and the user is not the superuser.
//        case EADDRNOTAVAIL : // A nonexistent interface was requested or the requested address was not local.
//        case EINVAL :  // The addrlen is wrong, or the socket was not in the AF_UNIX family.
//        case ENAMETOOLONG : // addr is too long.
//            syslog(LOG_CRIT, "Error encountered while trying to connect %s:%d : %s", current->address, current->port, strerror(result));
//            /** Giving up on this member */
//            //result = EINVAL;
//            pthread_exit((void*)&result);
//            /** this cannot happen */
//        case ENOTSOCK: // sockfd is a descriptor for a file, not a socket. The following errors are specific to UNIX domain (AF_UNIX) sockets:
//        case EFAULT: // addr points outside the user's accessible address space.
//        case ELOOP: // Too many symbolic links were encountered in resolving addr.
//        case ENOENT: // The file does not exist.
//        case ENOTDIR: // A component of the path prefix is not a directory.
//        case EROFS:// The socket inode would reside on a read-only file system.
//            syslog(LOG_EMERG, "Fatal error encountered while trying to connect %s:%d : %s", current->address, current->port, strerror(result));
//            pthread_exit((void*)&result);
//            /** notify and try reconnect */
//        case EADDRINUSE: // The given address is already in use.
//        case EBADF: //sockfd is not a valid descriptor.
//        case ENETUNREACH:
//        case ECONNREFUSED:
//            syslog(LOG_CRIT, "Error encountered while trying to connect %s:%d : %s", current->address, current->port, strerror(result));
//            sleep(reconnect_interval);
//            continue;
//            //pthread_exit((void*)&result);
//            /** @todo what to do with "Insufficient kernel memory was available" ?*/
//        case ENOMEM: // Insufficient kernel memory was available.
//            syslog(LOG_EMERG, "Fatal error on allocating kernel memory");
//            pthread_exit((void*)&result);
//        }

//        current->connected = 1;
//    }

//    pthread_exit((void*)&result);
}


void *rungpio_loop()
{
//    /** Initilize gpio array */
//    int i;
//    for(i = 0; i < gpio_array_size; ++i) {
//        if(pthread_create(&gpio_array[i]->connect_thread, NULL, connect_gpio, (void*)&i) != 0)
//        {
//            perror("Could not create server thread");
//            syslog(LOG_CRIT, "Could not create server thread");
//            exit(EXIT_FAILURE);
//        }
//    }

//    fd_set readfds;
//    int maxfd;

//    while (!shutdown_flag)
//    {
//        /** Prepare fdset for gpio pselect */
//        FD_ZERO(&readfds);
//        for(i = 0; i < gpio_array_size; ++i)
//            if(gpio_array[i]->connected == 1) {
//                maxfd = (maxfd<gpio_array[i]->sockfd)?gpio_array[i]->sockfd:maxfd;
//                FD_SET(gpio_array[i]->sockfd, &readfds);
//            }

//        int ready = pselect(maxfd + 1, &readfds, 0, 0, 0, 0);
//        /** @todo process/packet pack */
//        /** now just piping */

//        if (ready == -1) {
//            perror("select()");
//            /** Data from server */
//        } else if(ready)  // FD_ISSET(sockfd, &rfds)
//            for(i = 0; i < ready, i < gpio_array_size; ++i) {
//                if(FD_ISSET(gpio_array[i]->sockfd, &readfds))
//                {
//                    char data[1024] = {0};
//                    unsigned int len = 0;

//                    ioctl(gpio_array[i]->sockfd, FIONREAD, &len);

//                    /* Commented for dummy pipe */
//                    if(len == 0) {
//#ifndef DEBUG
//                        syslog(LOG_CRIT, "Malfunction empty read");
//                        exit(EXIT_FAILURE);
//#endif
//                    }

//#ifdef DEBUG
//                    printf("Data length: %d\n", len);
//                    printf("Size of buffer: %d\n", sizeof(data));
//#endif
//                    unsigned int readen;
//                    while(len > 0) {
//                        if(len > sizeof(data)) {
//                            readen = read(gpio_array[i]->sockfd, &data, sizeof(data));
//                        } else {
//                            readen = read(gpio_array[i]->sockfd, &data, len);
//                        }
//#ifdef DEBUG
//                        printf("Data: %s\n", data);
//#endif

//                        /** Dispach data to registered clients */
//                        int i = 0;
//                        for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i)
//                            if(client_pool[i].client_pid != -1) {
//#ifdef DEBUG
//                                printf("Writing to client pid: %d\t pipe_fd: %d\n", client_pool[i].client_pid, client_pool[i].write_fd);
//#endif
//                                write(client_pool[i].write_fd, &data, readen);
//                            }
//                        len -= readen;
//#ifdef DEBUG
//                        printf("Data left: %d\n", len);
//#endif
//                    }
//                    ready--;
//                }
//            }

//    } /* end of while */
}
