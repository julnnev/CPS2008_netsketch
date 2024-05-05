#include <iostream>
#include <cereal/archives/portable_binary.hpp>
#include "client.h"
char buffer[256];

void* writeToServer(void* arg){
    int sockfd = *static_cast<int *>(arg);
    int n;
    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255, stdin); //place message in buffer
    if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
    {
        fprintf(stderr,"ERROR writing to socket");
        close(sockfd);
        exit(1);
    }
    return nullptr;
}

void* readfromServer(void* arg){
    int n;
    int sockfd = *static_cast<int *>(arg);
    memset(buffer, 0, 256);
    if ((n = read(sockfd, buffer, 255)) < 0)
    {
        fprintf(stderr,"ERROR reading from socket");
        close(sockfd);
        exit(1);
    }
    return nullptr;
}

int main(int argc, char *argv[])
{
    // Declare variables
    int sockfd, portno;
    std::string nickname;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    // Make sure that server name and port are available in command line arguments
    if (argc < 3) {
        fprintf(stderr, "usage %s : hostname port nickname\n", argv[0]);
        exit(0);
    }
    // Get port number
    portno = atoi(argv[2]);
    //fprintf(stdout, argv[3]); debug
    nickname = argv[3];
    // Create a socket, similar to server
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        strerror(sockfd);
        fprintf(stderr,"ERROR opening socket");
        exit(1);
    }
    // Get server name
    // Given string representation of ip address, get the host by name
    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        close(sockfd);
        exit(0);
    }
    // Populate serv_addr structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //ipv4
    // Copy address given by gethostbyname into structure
    memcpy(&serv_addr.sin_addr.s_addr,
           server -> h_addr, // Set server address
           server -> h_length);
    serv_addr.sin_port = htons(portno); // Set port (convert to network byte ordering) provided by the client

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr,"ERROR connecting");
        close(sockfd);
        exit(1);
    }
//    else{ // try to connect using user name
//        ssize_t bytes_sent = send(sockfd, nickname.c_str(), nickname.size(), 0);
//        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
//        printf("%s\n", buffer);
//    }// else return message that client connected!


    pthread_t id_writeToServer;
    pthread_create(&id_writeToServer, NULL, writeToServer, &sockfd);
    pthread_join(id_writeToServer, nullptr);

    // in separate thread, Send message to the server
    pthread_t id_readfromServer;
    pthread_create(&id_readfromServer, NULL, readfromServer, &sockfd);
    pthread_join(id_readfromServer, nullptr);

    return 0;
}

// no: g++ -o client  client.cpp
// cd build
// gmake -j 8
// ./client 127.0.0.1 5001 ju


//snprintf(buffer, "%s", serialized_data.c_str());

//    Connect connect_request;
//    connect_request.username = nickname;
//    connect_request.success = false;
//    std::ostringstream oss;
//    // Create a BinaryOutputArchive object bound to oss
//    cereal::PortableBinaryOutputArchive archive(oss);
//    archive(connect_request);
//    std::string serialized_data = oss.str();
//serialise, send to server and check if given nickname valid (ie. unique)

// then proceed to accept instructions after printing success, otherwise exit


