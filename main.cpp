#include <raylib.h>
#include <iostream>
#include <string>
#include <thread>
#include <pthread.h>
#include <vector>
#include "draw.h"
#include "global_client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cereal/archives/portable_binary.hpp>

struct Head { //header object
public:
    size_t header;

    // serialization function for cereal
    template<class Archive>
    void serialize(Archive & archive) {
        archive(header);
    }

};

struct Connect {
public:
    std::string username;
    bool success;

    template<class Archive>
    void serialize(Archive & archive) {
        archive(username, success);
    }
};

struct ThreadArgs {
    // Draw* command;
    std::mutex *mutex;
    bool* cancelFlag;
};

void *readInput(void* arg) {
    auto *threadArgs = static_cast<ThreadArgs *>(arg);
    std::mutex *mutex = threadArgs->mutex;
    Draw command;
    bool* cancelFlag = threadArgs->cancelFlag;
    SetTraceLogLevel(LOG_WARNING); // hide LOG messages from raylib
    fflush(stdin);
    std::string input;
    std::tuple<int, int, int> RGBColour;
    auto colour = std::make_tuple(0,0,0); //default colour: black, in parsing - assuming strings will not be surrounded in " "
    std::string tool = "line"; //default tool:  line
    int select_id;
    bool select;
    int delete_id;
    std::string token;

    while (true) {
        std::vector<std::string> tokens;
        std::cout << ">>> " ;
        std::getline(std::cin, input);
        // debug: std::cout << "Command entered: " << input << std::endl;
        std::stringstream ss(input);

        while (ss >> token) {
            tokens.push_back(token);
        }

        if(tokens[0] == "exit"){
            std::cout << "Thank you for using netsketch, goodbye! "<< std::endl;
            *cancelFlag = true;
            // disconnect!
            break;
        } else if(tokens[0] == "help"){
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
            tokens.erase(std::remove(tokens.begin(), tokens.end(), "help"), tokens.end());
        }
        else if(tokens[0] == "tool"){
            std::transform(tokens[1].begin(), tokens[1].end(), tokens[1].begin(), [](unsigned char c) { return std::tolower(c); }); // Convert string to lowercase
            // std::cout << "Lowercase string: " << tokens[1] << std::endl;
            if(tokens[1] != "circle" && tokens[1] !=  "rectangle" && tokens[1] !=  "line" && tokens[1] !=  "text"){
                std::cout << " Unknown tool type! " << std::endl;
                // add more error handling for unknown tool types
            } else{
                tool = tokens[1];
            }

        }
        else if(tokens[0] == "colour"){
            try{
                colour = std::make_tuple(std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]));
            } catch (std::exception &err){
                std::cout << "Invalid parameters for colour!"<< std::endl;
            }

        }
        else if(tokens[0] == "draw"){
            try {

                if (tool == "text") {
                    TextShape text;
                    text.x = std::stoi(tokens[1]);
                    text.y = std::stoi(tokens[2]);

                    std::string text_input;
                    for (size_t i = 3; i < tokens.size(); ++i) {
                        text_input += tokens[i]; // Concatenate each string to the result
                        if (i < tokens.size() - 1) {
                            text_input += " "; // Retaining spaces
                        }
                    }
                    text.text = text_input;

                    {
                        std::scoped_lock lock{*mutex};
                        command.RGBColour = colour;
                        command.item = text;
                    }


                } else if (tool == "circle") {
                    CircleShape circleShape;
                    circleShape.x = std::stoi(tokens[1]);
                    circleShape.y = std::stoi(tokens[2]);
                    circleShape.radius = std::stof(tokens[3]);

                    {
                        std::scoped_lock lock{*mutex};
                        command.RGBColour = colour;
                        command.item = circleShape;
                    }

                } else if (tool == "rectangle") {
                    RectangleShape rectangleShape;
                    rectangleShape.topLeftX = std::stoi(tokens[1]);
                    rectangleShape.topLeftY = std::stoi(tokens[2]);
                    rectangleShape.bottomRightX = std::stoi(tokens[3]);
                    rectangleShape.bottomRightY = std::stoi(tokens[4]);

                    {
                        std::scoped_lock lock{*mutex};
                        command.RGBColour = colour;
                        command.item = rectangleShape;
                    }
                } else if (tool == "line") {
                    LineShape lineShape;
                    lineShape.startX = std::stoi(tokens[1]);
                    lineShape.endX = std::stoi(tokens[2]);
                    lineShape.startY = std::stoi(tokens[3]);
                    lineShape.endY = std::stoi(tokens[4]);

                    {
                        std::scoped_lock lock{*mutex};
                        command.RGBColour = colour;
                        command.item = lineShape;
                    }
                }

                if (select) {
                    {
                        std::scoped_lock lock{*mutex};
                        drawList[select_id].item = command.item;

                        // continue processing
                    }
                    select = false; //toggle back to off
                } else {
                    {
                        std::scoped_lock lock{*mutex};
                        drawList.push_back(command);

                        //continue processing
                    }
                }
            }catch (std::exception &err){
                std::cout << "Invalid parameters for draw!"<< std::endl;
            }


        }
        else if(tokens[0] == "list"){
            // define further if statements ...
            if(tokens[1] == "all" && tokens[2] == "all"){
                int index = 0;
                {
                    std::scoped_lock lock{*mutex};
                    for (Draw draw: drawList) {
                        std::cout << "[ " << index++ << " ] => ";
                        if (std::holds_alternative<CircleShape>(draw.item)) {
                            auto &circleShape = std::get<CircleShape>(draw.item);
                            std::cout << "[ circle ] " << "[ " << std::get<0>(draw.RGBColour) << ", "
                                      << std::get<1>(draw.RGBColour) << ", " << std::get<2>(draw.RGBColour) << " ] "
                                      << " [ " << circleShape.x << ", " << circleShape.y << ", " << circleShape.radius
                                      << " ]" << std::endl;
                        }
                        if (std::holds_alternative<RectangleShape>(draw.item)) {
                            auto &rectangleShape = std::get<RectangleShape>(draw.item);
                            std::cout << "[ rectangle ] " << "[ " << std::get<0>(draw.RGBColour) << ", "
                                      << std::get<1>(draw.RGBColour) << ", " << std::get<2>(draw.RGBColour) << " ] "
                                      << " [ " << rectangleShape.topLeftX << ", " << rectangleShape.topLeftY << ", "
                                      << rectangleShape.bottomRightX << ", " << rectangleShape.bottomRightY << " ]"
                                      << std::endl;
                        }
                        if (std::holds_alternative<LineShape>(draw.item)) {
                            auto &lineShape = std::get<LineShape>(draw.item);
                            std::cout << "[ line ] " << "[ " << std::get<0>(draw.RGBColour) << ", "
                                      << std::get<1>(draw.RGBColour) << ", " << std::get<2>(draw.RGBColour) << " ] "
                                      << " [ " << lineShape.startX << ", " << lineShape.startX << ", "
                                      << lineShape.startY << ", " << lineShape.endY << " ]" << std::endl;
                        }
                        if (std::holds_alternative<TextShape>(draw.item)) {
                            auto &textShape = std::get<TextShape>(draw.item);
                            std::cout << "[ text ] " << "[ " << std::get<0>(draw.RGBColour) << ", "
                                      << std::get<1>(draw.RGBColour) << ", " << std::get<2>(draw.RGBColour) << " ] "
                                      << " [ " << textShape.x << ", " << textShape.y << ", " << textShape.text << " ]"
                                      << std::endl;
                        }
                    }

                }
            }
        }

        else if(tokens[0] == "select"){
            if(tokens[1] == "none"){ //deselect
                select = false;
                select_id = -1;

            } else{
                try{
                    select_id = std::stoi(tokens[1]);
                    if (select_id >= 0 && select_id < drawList.size()) {
                        select = true;
                    } else{
                        std::cout << "Invalid id for select!"<< std::endl;
                        select = false;
                    }

                } catch(std::exception &err){
                    std::cout << "Invalid parameter for select!"<< std::endl;
                }
            }
        }

        else if (tokens[0] == "delete"){
            try {
                delete_id = std::stoi(tokens[1]);
                // output local list
                {
                    std::scoped_lock lock{*mutex};
                    if (delete_id >= 0 && delete_id < drawList.size()) {
                        drawList.erase(drawList.begin() + delete_id);
                    } else{
                        std::cout << "Invalid id for delete!"<< std::endl;
                    }
                }
            }

             catch(std::exception &err){
                std::cout << "Invalid parameter for delete!"<< std::endl;
            }

            // continue processing updating global list

        }
        else if (tokens[0] == "undo") {
            {
                std::scoped_lock lock{*mutex};
                if (!drawList.empty()) {
                    drawList.pop_back();
                } else{
                    std::cout << "No draws to undo!" << std::endl;
                }
            }
                // continue processing updating global list

        }
        else if (tokens[0] == "clear"){
            if(tokens[1] == "all"){

            } else if (tokens[1] == "mine"){

            } else{
                std::cout << "Invalid parameter for clear! " << std::endl;
            }

            // continue processing
            std::cout << "Continue ..." << std::endl;

        }
        else if (tokens[0] == "show"){
            if(tokens[1] == "all"){

            } else if (tokens[1] == "mine"){

            } else{
                std::cout << "Invalid parameter for show! " << std::endl;
            }

            // continue processing
            std::cout << "Continue ..." << std::endl;


        } else{
            std::cout << "Invalid command!\nSee help for list of commands"<< std::endl;
        }

//        Debug tokenizing
//        for (const auto& t : tokens) {
//            std::cout << t << std::endl;
//
//        }
//        for (size_t i = 0; i < tokens.size(); ++i) {
//            std::cout << "Token at index " << i << ": " << tokens[i] << std::endl;
//        }
    }
    return nullptr;
}


int main(int argc, char *argv[]) {
    int sockfd, portno;
    std::string nickname;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    // Ensuirng correct CLI arguments are given
    if (argc < 3) {
        fprintf(stderr, "usage %s : hostname port nickname\n", argv[0]);
        exit(0);
    }
    // Get port number
    portno = atoi(argv[2]);
    //fprintf(stdout, argv[3]); debug
    nickname = argv[3];

    // Socket creation
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        strerror(sockfd);
        fprintf(stderr,"ERROR opening socket");
        exit(1);
    }
    // Get server name - get the host by name given IP
    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        close(sockfd);
        exit(0);
    }
    //  serv_addr structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //ipv4
    memcpy(&serv_addr.sin_addr.s_addr,
           server -> h_addr, //  server address
           server -> h_length);
    serv_addr.sin_port = htons(portno); // port (convert to network byte ordering) provided by the client

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr,"ERROR connecting");
        close(sockfd);
        exit(1);
    }

    // write username to server
    std::string to_send = nickname + '\0';
    size_t payloadSize =  to_send.length();
    Head head = {};
    head.header = payloadSize;
    std::stringstream ss;
    cereal::PortableBinaryOutputArchive archive(ss);
    archive(head);
    std::string s;
    s = ss.str();
    //  send size to server
    int n;
    if ((n = write(sockfd, &s, 9)) < 0) {
        fprintf(stderr, "ERROR writing payload size to socket");
        close(sockfd);
        exit(1);
    }

    if ((n = write(sockfd, &nickname, payloadSize)) < 0) {
        fprintf(stderr, "ERROR writing payload content to socket");
        close(sockfd);
        exit(1);
    }

    char lclbuffer[256];
    bzero(lclbuffer, 256);
    n = read(sockfd, lclbuffer, 255);
    if (n < 0) {
        std::cerr << "ERROR reading response from socket\n";
        return 1;
    }
    std::cout  << lclbuffer << std::endl;

    std::mutex mutex;
    SetTraceLogLevel(LOG_WARNING); // hide LOG messages from raylib
    pthread_t thread_id;
    bool cancel=false;
    ThreadArgs threadArgs = {&mutex, &cancel};
    pthread_create(&thread_id, nullptr, readInput, &threadArgs);
    InitWindow(screenWidth, screenHeight, "netsketch");

    while (!cancel) {
        BeginDrawing();
        ClearBackground(WHITE);
        {
            std::scoped_lock lock{mutex};

            for (const auto& draw : drawList) {
                Color color = {static_cast<unsigned char>(std::get<0>(draw.RGBColour)),
                               static_cast<unsigned char>(std::get<1>(draw.RGBColour)),
                               static_cast<unsigned char>(std::get<2>(draw.RGBColour)), 255};
                if (std::holds_alternative<CircleShape>(draw.item)) {
                    auto &circleShape = std::get<CircleShape>(draw.item);
                    DrawCircle(circleShape.x, circleShape.y, circleShape.radius, color);
                }
                if (std::holds_alternative<RectangleShape>(draw.item)) {
                    auto &rectangleShape = std::get<RectangleShape>(draw.item);
                    int width = rectangleShape.bottomRightX - rectangleShape.topLeftX;
                    int height = rectangleShape.bottomRightY - rectangleShape.topLeftY;
                    int posX = (rectangleShape.topLeftX + rectangleShape.bottomRightX) / 2;
                    int posY = (rectangleShape.topLeftY + rectangleShape.bottomRightY) / 2;
                    DrawRectangle(posX, posY, width, height, color);
                }
                if (std::holds_alternative<LineShape>(draw.item)) {
                    auto &lineShape = std::get<LineShape>(draw.item);
                    DrawLine(lineShape.startX, lineShape.startY, lineShape.endX, lineShape.endY, color);
                }
                if (std::holds_alternative<TextShape>(draw.item)) {
                    auto &textShape = std::get<TextShape>(draw.item);
                    DrawText((textShape.text).c_str(), textShape.x, textShape.y, 30, color);
                }
            }
        }

        EndDrawing();
    }

    pthread_join(thread_id, nullptr);
    CloseWindow();
    if(cancel){
        close(sockfd);
    }

    return 0;
}
//cmake -S . -B build ; cd build ; make -j 8