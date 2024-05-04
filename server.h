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
#include "draw.h"
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <poll.h>
#include "connect_server.h"
#include "connect_server.cpp"
#include <ctime>

#define PORT 5001
#define MAXMSG  512

class server {

};

struct Command{
    Draw draw;
    int commandID;
    std::string author; //nickname/username of author
    bool show;
};

struct userData{
    std::string username;
    time_t lastLogged;
    std::vector<Command> authored;
};

struct ServerState{
    int clientsConnected = 0;
    std::vector<Command> commands;
    std::unordered_map <int, std::string> socketsToUsernames; //maps socket file descriptor to username
    std::unordered_map <std::string, userData> namesToData; //maps user data to username
};


#endif //NETSKETCH_SERVER_H
