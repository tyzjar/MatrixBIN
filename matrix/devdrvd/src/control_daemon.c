#include "control_daemon.h"
#include "command.h"

#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <err.h>

#include <errno.h>

#include "devdrvd.h"

extern volatile sig_atomic_t shutdown_flag;
static const char *msg_error = "Unknown command recieved.\n";
static const char* maximum_client_msg = "Maximum client size exceeded\r\n";
extern struct devdrvd_client client_pool[];
extern unsigned int clients_number;
extern pthread_mutex_t client_lock;

// int write_fd, int read_fd, int socket_fd
void* packet_feed(void*);

/*
 * socket_fd - клиентский сокет (matrix); write_fd, read_fd: (scaled,gpiod)
 */
int register_client(int write_fd, int read_fd, int socket_fd)
{
    int i = 0;
    for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i) {
        pthread_mutex_lock(&client_pool[i].lock);
        if(client_pool[i].socket_fd == -1) {
            /** Register new client */
            client_pool[i].socket_fd = socket_fd;
            client_pool[i].write_fd = write_fd;
            client_pool[i].read_fd = read_fd;
            pthread_mutex_lock(&client_lock);
            clients_number++;
            pthread_mutex_unlock(&client_lock);
            pthread_mutex_unlock(&client_pool[i].lock);


            if(pthread_create(&client_pool[i].control, NULL, packet_feed, (void *) (intptr_t) i) != 0)
            {
                perror("Could not create server thread");
                syslog(LOG_CRIT, "Could not create server thread");
                return EXIT_FAILURE;
            }

            return 0;
        }
        pthread_mutex_unlock(&client_pool[i].lock);
    }
    return 1;
}


void* packet_feed(void *contex)
{

    int id = (intptr_t)contex;
    int status = EXIT_SUCCESS;

    //syslog(LOG_INFO, "Client connected: %s", inet_ntoa(cli_addr.sin_addr));

    sigset_t mask, omask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, &omask));

    fd_set rfds;

    int retval;
    int max_fd;

    /** init here*/
    int *socket_fd = &client_pool[id].socket_fd;
    int *read_fd = &client_pool[id].read_fd;

    while(!shutdown_flag) {
        FD_ZERO(&rfds);

        FD_SET(*socket_fd, &rfds);
        FD_SET(*read_fd, &rfds);

        max_fd = (*socket_fd < *read_fd) ? *read_fd : *socket_fd;

        struct timespec tv;
        tv.tv_sec = 3;
        tv.tv_nsec = 0;

        retval = pselect(max_fd + 1, &rfds, 0, 0, &tv, &omask);
        int errsv = errno;

        //sigprocmask(SIG_SETMASK, &omask, 0);

        if (errsv == EINTR) {
            syslog(LOG_INFO, "Interrupted by SIGINT");
            continue;
        }
        if(retval == 0) {
            continue;
        } else if (retval == -1) {
            syslog(LOG_CRIT, "runcontrol pselect returned: %s", strerror(errsv));
            status = EXIT_FAILURE;
            goto quit;
        } else if (retval > 0) {
            if(FD_ISSET(*socket_fd, &rfds)) { // data from client
                char data[1024] = {0};
                int len = 0;

                ioctl(*socket_fd, FIONREAD, &len);

                if(len == 0) {
                    syslog(LOG_INFO, "Client diconnected");
                    goto quit;
                }

                if (len > 0) {
                    len = read(*socket_fd, &data, len);
                }
//#ifdef VERBOSE
				printf( "packet_feed\n");
                printf("Data length: %d\n", len);
                printf("Data: %s\n", data);
//#endif

                int n = write(*socket_fd, msg_error, strlen(msg_error));

                if (n < 0)
                {
                    syslog(LOG_CRIT, "ERROR writing to socket");
                    status = EXIT_FAILURE;
                    goto quit;
                }
            } else if(FD_ISSET(*read_fd, &rfds)) { // data from data server
                char data[1024] = {0};
                int len = 0;

                ioctl(*read_fd, FIONREAD, &len);

                if(len == 0) { // zero read from pipe
#ifdef DEBUG
                    printf("Data from server length: %d\n", len);
#endif

                    status = EXIT_FAILURE;
                    goto quit;
                }
//#ifdef VERBOSE
				printf( "packet_feed\n");
                printf("Data from server length: %d\n", len);
//#endif

                if(len > sizeof(data))
                    len = sizeof(data);

                if (len > 0) {
                    len = read(*read_fd, &data, len);
                }
//#ifdef VERBOSE
                printf("Data from server: %s\n", data);
//#endif

                int n = write(*socket_fd, data, len);
                if (n < 0)
                {
                    syslog(LOG_CRIT, "ERROR writing to socket");
                    status = EXIT_FAILURE;
                    goto quit;
                }
            }
        }
    }

quit:
    unregister_client(id);
    pthread_exit((void*)&status);
}

/**
 *
 * @brief runcontrol
 * @param contex
 * @return
 */

/*
 * Нить серверного сокета...
 * Ждет клиентского соединения, создает трубу
 */
void* runcontrol(void *contex)
{
    int pipefd[2];

    int sockfd = *((int*)contex);
    int newsockfd;
    int clilen;
    struct sockaddr_in cli_addr;
    int status = EXIT_SUCCESS;

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (!shutdown_flag)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        /** set non-blocking write */
        int flags;
        flags = fcntl(newsockfd, F_GETFL, 0);
        if(flags < 0)
        {
            syslog(LOG_CRIT, "Could not retrieve client socket flags.");
            status = EXIT_FAILURE;
            goto quit;
        }

        flags |= O_NONBLOCK;

        if(fcntl(newsockfd, F_SETFL, flags) < 0)
        {
            syslog(LOG_CRIT, "Could not set NON-BLOCKING flag for client socket.");
            status = EXIT_FAILURE;
            goto quit;
        }

        if(errno == EINVAL || errno == EINTR)
        {
            continue;
        } else if (newsockfd < 0) {
            perror("accept");
            syslog(LOG_CRIT, "ERROR on accept: %d", errno);
            status = EXIT_FAILURE;
            goto quit;
        }

        /** Initialize fifo server and launch for connections */
        /** Create pipe */
        if (pipe2(pipefd, O_NONBLOCK) == -1) {
            perror("pipe");
            syslog(LOG_CRIT, "Error on initializing pipes");
            status = EXIT_FAILURE;
            goto quit;
        }

        /** Create child process */
        if(register_client(pipefd[1], pipefd[0], newsockfd) != 0)
        {
            int ret = write(newsockfd, maximum_client_msg, sizeof(maximum_client_msg));
            ret = ret;
            close(newsockfd);
        }
    } /* end of while */

quit:
    close(sockfd);

    pthread_exit((void*)&status);
}
