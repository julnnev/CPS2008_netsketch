#include <iostream>
#include <cereal/archives/portable_binary.hpp>
#include "client.h"

int main(int argc, char *argv[])
{
    // Declare variables
    int sockfd, portno, n;
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

    // Populate nickname
    nickname = argv[3];
    // fprintf(stdout, argv[3]);

    // Create a socket, similar to server
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket");
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
        perror("ERROR connecting");
        close(sockfd);
        exit(1);
    }
//    else{ // try to connect using user name
//        ssize_t bytes_sent = send(sockfd, nickname.c_str(), nickname.size(), 0);
//        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
//        printf("%s\n", buffer);
//    }// else return message that client connected!

   // while(true){
        // Ask for a message from the user, if connected successfully
        printf("Please enter the message: ");
        memset(buffer, 0, 256);
        //fgets(buffer, 255, stdin); //place message in buffer

        //snprintf(buffer, "%s", serialized_data.c_str());

        // Replace newline with null terminator

    char *newline = strchr(buffer, '\n');
    if (newline != nullptr) {
        *newline = '\0';
    }

        // check for exit
        // !! important to close socket server side
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting...\n");
            close(sockfd);
            exit(0);
        }

    // check for help but don't send to server
    if (strcmp(buffer, "help") == 0) {
        printf("NetSketch: A Collaborative Whiteboard\n"
               "\n"
               "help - Lists all available commands and their usage.\n"
               "\n"
               "tool {line | rectangle | circle | text} - Selects a tool for drawing.\n"
               "\n"
               "colour {RGB} - Sets the drawing colour using RGB values.\n"
               "\n"
               "draw {parameters} - Executes the drawing of the selected shape on the canvas. For text, it requires coordinates and the text string. \n"
               "\n"
               "list {all | line | rectangle | circle | text} {all | mine} - Displays issued draw commands in the console.\n"
               "\n"
               "select {none | ID} - Selects an existing draw command to be modified by a subsequent draw command.\n"
               "\n"
               "delete {ID} - Deletes the draw command with the specified ID.\n"
               "\n"
               "undo - Reverts your last draw action.\n"
               "\n"
               "clear {all | mine} - Clears the canvas. Using the argument ‘mine‘ clears only the draw commands issued by you; without it, the entire canvas is cleared.\n"
               "\n"
               "show {all | mine} - Controls what is displayed on the your canvas. Specifying ‘mine‘ will display only the commands issued by you.\n"
               "\n"
               "exit - Disconnects and exits the application.\n");

    }

    Connect connect_request;
    connect_request.username = nickname;
    connect_request.success = false;
    std::ostringstream oss;
    // Create a BinaryOutputArchive object bound to oss
    cereal::PortableBinaryOutputArchive archive(oss);
    archive(connect_request);
    std::string serialized_data = oss.str();
    //serialise, send to server and check if given nickname valid (ie. unique)

    // then proceed to accept instructions after printing success, otherwise exit

        // Send message to the server
        //if ((n = write(sockfd, serialized_data.data(), strlen(serialized_data.size()))) < 0)
        if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
        {
            perror("ERROR writing to socket");
            close(sockfd);
            exit(1);
        }

        // Read response from server response
        memset(buffer, 0, 256);
        if ((n = read(sockfd, buffer, 255)) < 0)
        {
            perror("ERROR reading from socket");
            close(sockfd);
            exit(1);
        }

        printf("%s\n", buffer);
    //}

    return 0;
}

// no: g++ -o client  client.cpp

// cd build
//gmake -j 8
// ./client 127.0.0.1 5001 ju

