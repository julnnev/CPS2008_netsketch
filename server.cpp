#include <sstream>
#include "server.h"

struct toRead{
    int* p_filedes;
    fd_set* active_fd_set;
    bool* close_socket;
};

struct Head { //struct, header object, its size is hard coded in the first write/read operations before payload sent
public:
    size_t header;

    // Serialization function
    template<class Archive>
    void serialize(Archive & archive) {
        archive(header);
    }
};


// one thread for each client that waits for client input
void* readFromClient(void* args){
    auto* info = static_cast<toRead*>(args);
    int filedes = *info->p_filedes;
    fd_set active_fd_set = *info -> active_fd_set;
    int nbytes;

    std::vector<char> header;
    header.reserve(9);
    nbytes = read(filedes, header.data(), 9);
    //insert error handling checking value of nbytes
    if(nbytes<0){
        fprintf(stderr, "ERROR reading payload size from socket");
        close(filedes);
        exit(EXIT_FAILURE);
    }

    std::string header_str;

    for (int i = 0; i < 9; i++) {
        header_str.push_back(header[i]);
    }

    // deserialize the string
    std::istringstream iss(header_str);
    Head head = {};
    {
        cereal::PortableBinaryInputArchive archive(iss);
        archive(head);
    }

    size_t payloadSize = head.header;
    std::vector<char> buffer(payloadSize);
    buffer.reserve(payloadSize);

    char lclbuffer[payloadSize];
    nbytes = read(filedes, lclbuffer, payloadSize);
    // DEBUG: std::cout  << nbytes << std::endl;

    if (nbytes < 0) {
        // Read error
        fprintf(stderr, "ERROR reading payload content from socket");
        close(filedes);
        exit(EXIT_FAILURE);
    }
    else if (nbytes == 0) { //nothing read from client
        *info->close_socket = true; //closes socket
        FD_CLR (filedes, &active_fd_set);
        close(filedes);
    }
    else {
        std::cout <<  "Server got: " << lclbuffer << std::endl;
        const char* message_response = "Connected to Netsketch Server!\nReady to draw.\nType help for command list.";
        size_t length = strlen(message_response);
        int n;

        if ((n = write(filedes, message_response, length)) < 0) {
            fprintf(stderr, "ERROR writing to socket");
        }

//        std::cout << "Buffer content: " << buffer << std::endl;
//        std::cout << "nbytes =  " << nbytes << std::endl;
//
//        // Data read.
//        std::istringstream iss(std::string(buffer, sizeof(buffer)));
//        Connect connect;
//        // Deserialize the data from the input string stream
//        {
//            cereal::PortableBinaryInputArchive archive(iss);
//            archive(connect1);
//        }
//
//        fprintf(stdout, "Server: got message: `%s`", connect1.username.c_str());

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
    }
    return nullptr;
}

void* handleConnections(void *arg){
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

    // Set of Active Sockets
    FD_ZERO (&active_fd_set); // set to all zeros
    FD_SET (sock, &active_fd_set); // when an action is pending, call accept

    while (true) {
        // Blocking until input arrives.
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, nullptr, nullptr, nullptr) < 0) {
            fprintf(stderr,"select");
            close(sock);
            exit(EXIT_FAILURE);
        }

        // When select returns, service pending sockets
        for (int i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET (i, &read_fd_set)) {
                if (i == sock) { //original socket?
                    int New;
                    size = sizeof(clientname);

                    // new socket bound to the client
                    New = accept(sock, (struct sockaddr *) &clientname, reinterpret_cast<socklen_t *>(&size));

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
                    // at this point our set contains the original new sockets

                    // check for unique username and store in list

                } else {
                    // Data on connected socket.
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
    Head header = {};
    std::stringstream ss;
    cereal::PortableBinaryOutputArchive archive(ss);
    archive(header);
    std::string s;
    s = ss.str();
    std::size_t serialized_size = s.length();
  //  std::cout << "Size of serialized object: " << serialized_size << " bytes" << std::endl; //9 bytes
    ServerState state;
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
    pthread_create(&thread_id_handleconnections, nullptr, handleConnections, &sock);
    pthread_join(thread_id_handleconnections, nullptr);
}
// thread to update all clients
//void* writeToClient(void* args){
//
//}