#ifndef NETSKETCH_SERVER_H
#define NETSKETCH_SERVER_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <poll.h>
#include "connect_server.h"
#include "connect_server.cpp"

#define PORT 5001
#define MAXMSG  512

class server {

};


#endif //NETSKETCH_SERVER_H
