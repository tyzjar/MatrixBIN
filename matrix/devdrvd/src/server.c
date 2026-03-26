#include "server.h"


// GLOBAL INCLUDES
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>

#if defined(__linux__)
    #include <linux/sockios.h>
#endif

// PROJECT INCLUDES
#include "command.h"
#include "control_daemon.h"

static const char *msg_greet = "Welcome to scale emulator daemon control.\n Enter: help,? for usage.\n";
volatile sig_atomic_t got_interrupted = 0;

void greet_control(int sock)
{
    int n = write(sock,msg_greet,strlen(msg_greet));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void print(int sock)
{
    printf("Sending message: %s\n", msg_greet);
    int n = write(sock, msg_greet, strlen(msg_greet));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void start(char* input, int sockfd)
{
    input = input;
    char *msg_start = "Start function called!\n";
    int n = write(sockfd, msg_start, strlen(msg_start));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void stop(char* input, int sockfd)
{
    input = input;
    char *msg_stop = "Stop function called!\n";
    int n = write(sockfd, msg_stop, strlen(msg_stop));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void load(char* input, int sockfd)
{
    input = input;
    char *msg_load = "Load function called!\n";
    int n = write(sockfd, msg_load, strlen(msg_load));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void __shutdown(char* input, int sockfd)
{
    input = input;
    char *msg_shutdown = "Shutdown function called!\n";
    int n = write(sockfd, msg_shutdown, strlen(msg_shutdown));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
    got_interrupted = 1;
}

int init_socket(const char* ipaddr, int portno)
{
    struct sockaddr_in addr;
    /* First call to socket() function */
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
      switch (errno)
      {
        case EPROTONOSUPPORT:
          break;
        case EMFILE:
          break;
        case ENFILE:
          break;
        case EACCES:
          break;
        case ENOBUFS:
          break;
        case EINVAL:
          break;
        default:
          break;
      }
      perror("Socket open failure.\n");
      exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = PF_INET;
    if(ipaddr == 0)
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        addr.sin_addr.s_addr = inet_addr(ipaddr);

    addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    return sockfd;
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
    if (argc >=1) {
        printf("Usage: %s -n -i [host] -p [port]\n", argv[0]);
        printf("  Options:\n");
        printf("      -n\tDon't fork off as a daemon\n");
        printf("      -i\tSet ip address for usage\n");
        printf("      -p\tSet port number for usage\n");
        printf("      -h\tShow this help screen\n");
        printf("n");
    }
}

/**************************************************************************
    Function: init_commands

    Description:
        Place your own commands for the daemon here

    Params:

    Returns:
        returns void always

**************************************************************************/
void init_commands()
{
    register_command("^start$", start);
    register_command("^stop$", stop);
    register_command("^load$", load);
    register_command("^shutdown$", __shutdown);
}

void caught_signal(int sig) {
    switch(sig) {
        case SIGTERM:
            //syslog(LOG_WARNING, "Received SIGTERM signal.");
            //syslog(LOG_INFO, "Cleaning and shutting down.");
            printf("Received SIGTERM signal.\n");
            printf("Cleaning and shutting down.\n");
            got_interrupted = 1;
            break;
        case SIGINT:
            printf("Received SIGINT signal.\n");
            printf("Cleaning and shutting down.\n");
            got_interrupted = 1;
            break;
        default:
            printf("Unhandled signal (%d) %s", strsignal(sig));
            break;
    }
}

int main( int argc, char *argv[] )
{
    struct sigaction sa;

    sa.sa_handler = caught_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* Restart functions if
                                 interrupted by handler */
    if (sigaction(SIGINT, &sa, NULL) == -1)
        /* Handle error */;

#if defined(DEBUG)
    int daemonize = 0;
#else
    int daemonize = 1;
#endif
    int c;

    char *ipaddr = 0; // ip adress for open
    int listen_portno = 1500;
    int control_portno = 1501;

    while( (c = getopt(argc, argv, "hni:p:с:")) != -1) {
        switch(c){
            case 'h':
                PrintUsage(argc, argv);
                exit(0);
                break;
            case 'n':
                daemonize = 0;
                break;
            case 'i':
                ipaddr = optarg;
                break;
            case 'p':
                listen_portno = atoi(optarg);
                if(listen_portno == 0) {
                    perror("Please give me corrent port number.\n");
                    exit(0);
                }
                break;
            case 'c':
                control_portno = atoi(optarg);
                if(control_portno == 0) {
                    perror("Please give me correct port number.\n");
                    exit(0);
                }
            break;
            default:
                PrintUsage(argc, argv);
                exit(0);
                break;
        }
    }

    int     server_sockfd,  // server listen socket
            control_sockfd; // control listen socket

    /* Initializing daemon control socket
     */
    control_sockfd = init_socket(ipaddr, control_portno);

    /* Initializing server socket
     */
    server_sockfd = init_socket(ipaddr, listen_portno);

    /* Initializing daemon control
     */
    init_commands();

    /* Now start listening for the clients, here
     * process will go in sleep mode and will wait
     * for the incoming connection
     */

    pthread_t control;
    if(pthread_create(&control, NULL, runcontrol, control_sockfd) != 0){
        perror("Could not create server thread");
        return EXIT_FAILURE;
    }

    pthread_join(control, NULL);
}
