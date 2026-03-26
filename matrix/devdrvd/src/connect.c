#include "connect.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>

#include "devdrvd.h"

int init_listen_socket(const char* ipaddr, const int portno, int *sockfd)
{

    struct sockaddr_in addr;
    int result = 0;
    /* First call to socket() function */
    *sockfd = socket(PF_INET, SOCK_STREAM, 0);

    int on, ret;

    on = 1;
    ret = setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    ret = ret;

#ifdef DEBUG
    printf("init_listen_socket(%s, %d, %d);\n", ipaddr, portno, *sockfd);
#endif

    if (*sockfd == -1)
    {
        result = errno;
        return result;
    }

    /* Initialize socket structure */
    bzero((char *) &addr, sizeof(addr));

    addr.sin_family = PF_INET;

    if(ipaddr == 0)
        addr.sin_addr.s_addr = INADDR_ANY;
    else {
        addr.sin_addr.s_addr = inet_addr(ipaddr);
        if(addr.sin_addr.s_addr == INADDR_NONE) {
            syslog(LOG_DEBUG, "Unrecognized address: %s", ipaddr);
            addr.sin_addr.s_addr = INADDR_LOOPBACK;
        }
    }

    addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if(bind(*sockfd, (struct sockaddr *) &addr, sizeof(addr)) != 0)
        result = errno;

    return result;
}

int init_socket()
{

}

/* 
 * Нити установления клиентских соединений к серверам устройств - поставщиков данных
 */
void* connect_client(void *contex)
{
    extern volatile sig_atomic_t shutdown_flag;
    //extern pthread_mutex_t client_lock;
    extern pthread_mutex_t connect_lock;


    int result = 0;    

    struct devdrvd_connect* current = ((struct devdrvd_connect*)contex);

    pthread_mutex_lock(&connect_lock);
    current->connected = 0;
    current->errsv = 0;
    pthread_mutex_unlock(&connect_lock);

    if(current == 0)
    { //fatal
        syslog(LOG_CRIT, "Called connect on corrupt member.");
        pthread_exit(0);
    }

    while(shutdown_flag != 1 || current->connected != 1) {
        pthread_mutex_lock(&connect_lock);
        /** array element can be wiped out so check */
        current->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(current->port);
        serv_addr.sin_addr.s_addr = inet_addr(current->address);

        syslog(LOG_INFO, "Connecting to client %s:%d", current->address, current->port);

        result = connect(current->sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        if(result == 0) {
            /* Set the option active */
            int optval = 1;
            socklen_t optlen = sizeof(optval);

            if(setsockopt(current->sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
                  perror("setsockopt()");
                  close(current->sockfd);
                  exit(EXIT_FAILURE);
            }

            current->connected = 1;
            syslog(LOG_INFO, "Successfully connected to client %s:%d", current->address, current->port);
            pthread_mutex_unlock(&connect_lock);            
            pthread_exit(0);
        }
        pthread_mutex_unlock(&connect_lock);

        current->errsv = errno;
        switch(current->errsv) {
        /** nothing can be done: */
        case EACCES : //The address is protected, and the user is not the superuser.
        case EPERM:
        case EADDRNOTAVAIL : // A nonexistent interface was requested or the requested address was not local.
        case EINVAL :  // The addrlen is wrong, or the socket was not in the AF_UNIX family.
        case ENAMETOOLONG : // addr is too long.
        case EAFNOSUPPORT:
            syslog(LOG_CRIT, "Error encountered while trying to connect %s:%d : %s", current->address, current->port, strerror(errno));
            /** Giving up on this member */
            pthread_exit(0);
            /** this cannot happen */
        case ENOTSOCK: // sockfd is a descriptor for a file, not a socket. The following errors are specific to UNIX domain (AF_UNIX) sockets:
        case EFAULT: // addr points outside the user's accessible address space.
        case ELOOP: // Too many symbolic links were encountered in resolving addr.
        case ENOENT: // The file does not exist.
        case ENOTDIR: // A component of the path prefix is not a directory.
        case EROFS:// The socket inode would reside on a read-only file system.
            syslog(LOG_EMERG, "Fatal error encountered while trying to connect %s:%d : %s", current->address, current->port, strerror(errno));
            pthread_exit(0);
            /** notify and try reconnect */
        case EADDRINUSE: // The given address is already in use.
        case EBADF: //sockfd is not a valid descriptor.
        case ENETUNREACH:
        case ECONNREFUSED:
            syslog(LOG_CRIT, "Error encountered while trying to connect %s:%d : %s", current->address, current->port, strerror(errno));
            sleep(reconnect_interval);
            continue;
            /** @todo what to do with "Insufficient kernel memory was available" ?*/
        case ENOMEM: // Insufficient kernel memory was available.
            syslog(LOG_EMERG, "Fatal error on allocating kernel memory");
            pthread_exit(0);
        }
    }
    pthread_exit(0);
}
