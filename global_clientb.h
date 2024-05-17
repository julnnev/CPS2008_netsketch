//
// Created by Julianne Vella on 11/05/2024.
//

#ifndef NETSKETCH_GLOBAL_CLIENTB_H
#define NETSKETCH_GLOBAL_CLIENTB_H
#include <unistd.h>
#include <string>

class global_clientb {

};

extern std::vector<char> buffer;
extern std::atomic<bool> cancel;

#endif //NETSKETCH_GLOBAL_CLIENTB_H
