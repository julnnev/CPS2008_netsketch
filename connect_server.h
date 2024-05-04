//
// Created by Julianne Vella on 29/04/2024.
//

#ifndef NETSKETCH_CONNECT_SERVER_H
#define NETSKETCH_CONNECT_SERVER_H
#include <cereal/types/string.hpp>
struct Connect { //change to struct
public:
    std::string username;
    bool success;

    // Serialization function
    template<class Archive>
    void serialize(Archive & archive) {
        archive(username, success);
    }
};


#endif //NETSKETCH_CONNECT_SERVER_H
