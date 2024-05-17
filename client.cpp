#include <iostream>
#include <cereal/archives/portable_binary.hpp>
#include "client.h"
#include "global_clientb.h"

struct threadData{
    int* sockfd;
    std::mutex *mutex;
};

struct Head { //struct, header object
public:
    size_t header;

    // Serialization function
    template<class Archive>
    void serialize(Archive & archive) {
        archive(header);
    }

};

void* readfromServer(void* arg){
    int n;
    std::vector<char> buffer = {};
    auto *threadArgs = static_cast<threadData *>(arg);
    int sockfd = *threadArgs->sockfd;
    char ackBuffer[1024];
    std::mutex *mutex = threadArgs->mutex;

   // buffer.clear();

//    while(true){
//        n = recv(sockfd, &ackBuffer, 1024, 0);   // why is n=0?
//
//        if (n < 0)
//        {
//            std::cout << "Connection closed by server. \n"<< std::endl;
//            break;
//        }
//
//        //if(n!=0){
//            ackBuffer[n] = '\0';
//         //   std::cout << n << std::endl;
//         //   fprintf(stdout, "Client got: %s", buffer.data());
//            std::cout << "Acknowledgment from server: " << ackBuffer << std::endl;
//        //}
//
//    }



    while(true){
        std::vector<char> header;
        header.reserve(9);
        n = read(sockfd, header.data(), 9);
        // error checking on n

        std::string header_str;
        for (char c : header) {
            header_str += c;
        }

        // deserialize the string
        std::istringstream iss(header_str);
        Head head = {};
        {
            cereal::PortableBinaryInputArchive archive(iss);
            archive(head);
        }

        size_t  payloadSize = head.header;
        {
            std::scoped_lock lock{*mutex};
            buffer.reserve(payloadSize);
            n = read(sockfd, &buffer, payloadSize);
        }


        if(n!=0){
            std::cout << "Acknowledgment from server: ";
            for(int i = 0; i<payloadSize;i++){
                {
                    std::scoped_lock lock{*mutex};
                    if (buffer[i] >= 32 && buffer[i] <= 126) { // Check if the character is printable
                        std::cout << buffer[i];
                    }
                }

            }
            fprintf(stdout, "\n");  // Print a newline at the end
           // fprintf(stdout, "Client got: %s", buffer.c_str()r);
           // buffer.clear();
        //std::cout << "Acknowledgment from server: " << buffer << std::endl;
        } else{
            fprintf(stderr,"ERROR reading from socket client");
             close(sockfd);
             exit(1);
        }

       // buffer.clear();


    }

//    while(true){
//        n = read(sockfd, &buffer[0], 18);   // why is n=0?
//        //std::cout << n << std::endl;
//
//        if (n < 0)
//        {
//            std::cout << "Connection closed by server. \n"<< std::endl;
//            break;
//        }
//
//        if(n!=0){
//        //    fprintf(stdout, "Client got: %s", buffer.data());
//            std::cout << "Acknowledgment from server: " << buffer << std::endl;
//        }
//    }
    //fprintf(stderr,"ERROR reading from socket client");
   // close(sockfd);
   // exit(1);




}

void* writeToServer(void* arg) {
    std::atomic<bool> cancel = false;
    auto *threadArgs = static_cast<threadData *>(arg);
    std::mutex *mutex = threadArgs->mutex;
    int sockfd = *threadArgs->sockfd;
    char ackBuffer[1024];

    std::string cancel_str = "exit";
    int n;

    while(!cancel){
        std::vector<char> buffer;
        printf("Please enter the message: ");

        char c;
        {
            std::scoped_lock lock{*mutex};
            while (std::cin.get(c) && c != '\n') {
                buffer.push_back(c);
            }
        }
        buffer.push_back('\0');


      // string to send
        std::string to_send;
        {
            std::scoped_lock lock{*mutex};
            for (char c: buffer) {
                to_send += c;
            }
        }
        std::cout<< to_send;

        size_t payloadSize =  to_send.length();

        Head head = {};
        head.header = payloadSize;
        std::stringstream ss;
        cereal::PortableBinaryOutputArchive archive(ss);
        archive(head);
        std::string s;
        s = ss.str();


   //  send size to server
    if ((n = write(sockfd, &s, 9)) < 0) {
        fprintf(stderr, "ERROR writing payload size to socket");
        close(sockfd);
        exit(1);
    }

    //  send actual payload
        {
            std::scoped_lock lock{*mutex};
            if ((n = write(sockfd, &buffer, payloadSize)) < 0) {
                fprintf(stderr, "ERROR writing to socket");
                close(sockfd);
                exit(1);
            }
        }


        {
            std::scoped_lock lock{*mutex};
            auto it = std::search(buffer.begin(), buffer.end(), cancel_str.begin(), cancel_str.end());
            // Check if the target string was found
            if (it != buffer.end()) {
                cancel = true;
            }
        }
       // printf("done writing message");

   }
    // send payload to indicate exit to server so that it can close the socket of the particular client
  //  close(sockfd);
   // exit(0);

    return nullptr;

//    Connect connect_request;
//    connect_request.username = "user1";
//    connect_request.success = false;
//    std::ostringstream oss;
//    // Create a BinaryOutputArchive object bound to oss
//    cereal::PortableBinaryOutputArchive archive(oss);
//    archive(connect_request);
//    std::string serialized_data = oss.str();
   // std::copy(serialized_data.begin(), serialized_data.end(), buffer);
//    serialise, send to server and check if given nickname valid (ie. unique)


   // fgets(buffer, 255, stdin); //place message in buffer
   // if ((n = write(sockfd, buffer, serialized_data.size())) < 0)


}



int main(int argc, char *argv[])
{
   // std::string buffer = {};
    // Declare variables
    int sockfd, portno;
    std::string nickname;
    struct sockaddr_in serv_addr;
    struct hostent *server;
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

    // check with username
//    Connect connect_request;
//    connect_request.username = nickname;
//    connect_request.success = false;
//    std::ostringstream oss;
    // Create a BinaryOutputArchive object bound to oss
//    cereal::PortableBinaryOutputArchive archive(oss);
//    archive(connect_request);
//    std::string serialized_data = oss.str();

  //  int n;
    //int payload_size = htonl(buffer.length());
//    if ((n = write(sockfd, &serialized_data, 4)) < 0) {
//        fprintf(stderr, "ERROR writing to socket");
//        close(sockfd);
//        exit(1);
//    }

   // std::copy(serialized_data.begin(), serialized_data.end(), buffer);
   // buffer.assign(serialized_data);

    //  send actual payload
//    if ((n = write(sockfd, &buffer, buffer.length())) < 0) {
//        fprintf(stderr, "ERROR writing to socket");
//        close(sockfd);
//        exit(1);
//    }


//    else{ // try to connect using user name
//        ssize_t bytes_sent = send(sockfd, nickname.c_str(), nickname.size(), 0);
//        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
//        printf("%s\n", buffer);
//    }// else return message that client connected!


    pthread_t id_writeToServer;
    pthread_t id_readfromServer;
    std::mutex mutex;
    threadData data = {&sockfd, &mutex};
    pthread_create(&id_readfromServer, NULL, readfromServer, &data);
    pthread_create(&id_writeToServer, NULL, writeToServer, &data);
    pthread_join(id_readfromServer, nullptr);
    pthread_join(id_writeToServer, nullptr);

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
//    serialise, send to server and check if given nickname valid (ie. unique)

// then proceed to accept instructions after printing success, otherwise exit


