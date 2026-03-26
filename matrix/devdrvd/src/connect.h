#ifndef CONNECT_H
#define CONNECT_H

#include "devdrvd.h"

int init_listen_socket(const char*, int, int *);
void* connect_client(void *);

#endif
