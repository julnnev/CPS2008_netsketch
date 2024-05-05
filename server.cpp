#include <sstream>
#include <cereal/archives/portable_binary.hpp>
#include "server.h"
#include <pthread.h>

struct toRead{
    int* p_filedes;
    fd_set* active_fd_set;
    bool* close_socket;
};

void* writeToClient(void* args){

}
// one thread for each client that waits for client input
void* readFromClient(void* args){
    auto* info = static_cast<toRead*>(args);
    int filedes = *info->p_filedes;
    fd_set active_fd_set = *info -> active_fd_set;
    char buffer[MAXMSG];
    int nbytes;

    nbytes = read(filedes, buffer, MAXMSG);

    if (nbytes < 0) {
        // Read error
        perror("read");
        close(filedes);
        exit(EXIT_FAILURE);
    }
    else if (nbytes == 0) {//nothing read from client
        // End-of-file.
        *info->close_socket = true; //closes socket
//        close(filedes);
//        FD_CLR (filedes, &active_fd_set);
    }
    else {
        // Data read.
        fprintf(stdout, "Server: got message: `%s`", buffer);
        if ((nbytes = write(filedes, "I got your message", 18)) < 0) {
            close(filedes);
            perror("ERROR writing to socket");
            exit(EXIT_FAILURE);
        }

//        Connect connection;
//        std::istringstream iss(std::string(buffer, MAXMSG));
//        cereal::PortableBinaryInputArchive archive(iss);
//        archive(connection); // connection is the object
//
//        connection.username;
//        connection.success;
//
//        // Data read.
//        fprintf(stderr, "Server: got success: `%d`\n", connection.success);
//        fprintf(stderr, "Server: got message: `%s`\n", connection.username.c_str());
    }
    return nullptr;
}

void* handleConnections(void *arg){
    int socket;
    fd_set active_fd_set, read_fd_set;
    int i;
    struct sockaddr_in clientname;
    int size;
    int sock = *static_cast<int *>(arg);

    // Listen for connections
    if (listen(sock, 1) < 0) {
        fprintf(stderr,"listen");
        close(sock);
        exit(EXIT_FAILURE);
    }

    /* Initialize the set of active sockets. */
    FD_ZERO (&active_fd_set); //set to all zeros
    FD_SET (sock, &active_fd_set); // if there is an action pending on this set, we need to call accept on that socket

    while (true) { //infinite loop
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, nullptr, nullptr, nullptr) < 0) {
            fprintf(stderr,"select");
            close(sock);
            exit(EXIT_FAILURE);
        }

        // When select returns we need to service sockets with pending action.
        /* Service all the sockets with input pending. */

        for (int i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET (i, &read_fd_set)) { // is bit set/not, ie. 1 pending, 0 none pending
                if (i == sock) { //original socket?
                    /* Connection request on original socket. */
                    int New;
                    size = sizeof(clientname);

                    New = accept(sock, (struct sockaddr *) &clientname, reinterpret_cast<socklen_t *>(&size)); // new socket bound to the client

                    if (New < 0) {
                        close(sock);
                        fprintf(stderr,"accept");
                        exit(EXIT_FAILURE);
                    }

                    fprintf(stderr,
                            "Server: connect from host %s, port %d.\n",
                            inet_ntoa(clientname.sin_addr),
                            ntohs(clientname.sin_port));

                    FD_SET (New, &active_fd_set); // we need to add new socket to active file descriptor set
                    // at this point our set contains the original and the new socket (2)

                    // get connect packet
                    // deserialize
                    // check for unique username, otherwise close connection and notify client
                    //

                    // update server state

                } else {
//                    char buffer[1024];
//                    ssize_t bytes_received = recv(i, buffer, sizeof(buffer), 0);
//                    if (bytes_received == -1) {
//                        perror( "Error receiving data from client");
//                        close(i);
//                        FD_CLR (i, &active_fd_set);
//                    }
//
//                    buffer[bytes_received] = '\0';
//
//                    // Check if the received username is valid
//                    string username(buffer);
//                    bool exists = false;
//                    for (const auto& user : users) {
//                        if (username == user) {
//                            exists = true;
//                            break;
//                        }
//                    }
//
//                    string response;
//                    if (!exists) {
//                        response = "Welcome, " + username;
//                    } else {
//                        response = "User with given nickname already exists";
//                    }
//                    if (send(i, response.c_str(), response.size(), 0) == -1) {
//                        perror("Error sending data to client");
//                    }
//                    if (exists) {
//                        // Close the client socket for invalid usernames
//                        close(i);
//                        FD_CLR (i, &active_fd_set);
//                        continue;
//                    }

                    /* Data arriving on an already-connected socket. */
                    // put in seperate thread
                    pthread_t thread_id_readFromClient;
                    bool close_socket = false;
                    toRead info = {&i, &active_fd_set, &close_socket};
                    pthread_create(&thread_id_readFromClient, nullptr, readFromClient, &info);
                    pthread_join(thread_id_readFromClient, nullptr);

                    if (close_socket) {
                        close(i);
                        FD_CLR (i, &active_fd_set);
                    }
                }
            }
    }
}

int main() {
    ServerState state;
    // Declare variables
    int sock;
    struct sockaddr_in serv_addr;

    // Create server socket (AF_INET, SOCK_STREAM)
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"ERROR opening socket");
        exit(1);
    }

    // Allow address reuse
    int opts = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));

    // Get current options
    if ((opts = fcntl(sock, F_GETFL)) < 0) {
        fprintf(stderr,"Error getting socket options\n");
        close(sock);
        exit(1);
    }

    // Set socket to non-blocking
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        {
            fprintf(stderr,"Error setting socket to non-blocking");
            close(sock);
            exit(1);
        }
    }

    // Initialize socket structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address
    serv_addr.sin_port = htons(PORT);

    // Bind the host address
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr,"ERROR on binding");
        close(sock);
        exit(1);
    }

    pthread_t thread_id_handleconnections;
    pthread_create(&thread_id_handleconnections, NULL, handleConnections, &sock);
    pthread_join(thread_id_handleconnections, nullptr);
}

//  g++ -o server server.cpp
// ./server


// thread to update all clients