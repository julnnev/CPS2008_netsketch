#include <raylib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>

struct CircleShape {
    std::string name = "circle";
    int x;
    int y;
    float radius;
};

struct RectangleShape  {
    std::string name = "rectangle";
    int topLeftX;
    int topLeftY;
    int bottomRightX;
    int bottomRightY;
};

struct LineShape{
    std::string name = "line";
    int startX{};
    int startY{};
    int endX{};
    int endY{};
};

struct TextShape{
    std::string name = "text";
    int x;
    int y;
    std::string text;
};

using Item = std::variant<CircleShape, RectangleShape, LineShape, TextShape>;

struct Command {
    Item item;
    std::tuple<int, int, int> RGBColour;
};

void readInput(Command *command, std::mutex& mutex) {
    fflush(stdin);
    //Command drawCommand;
    std::string input;
    std::tuple<int, int, int> RGBColour;
    auto colour = std::make_tuple(0,0,0); //default colour is black
    std::string tool = "line"; //default line

    //in parsing - assuming strings will not be surrounded in " "

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
            std::cout << "Goodbye! "<< std::endl;
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
            std::transform(tokens[1].begin(), tokens[1].end(), tokens[1].begin(), [](unsigned char c) { return std::tolower(c); });
            // Convert string to lowercase std::cout << "Lowercase string: " << tokens[1] << std::endl;
            if(tokens[1] != "circle" && tokens[1] !=  "rectangle" && tokens[1] !=  "line" && tokens[1] !=  "text"){
                std::cout << " Unknown tool type! " << std::endl;
                // add more error handling for unknown tool types
            } else{
                tool = tokens[1];
            }

        }
        else if(tokens[0] == "colour"){
            colour = std::make_tuple(std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]));
        }
        else if(tokens[0] == "draw"){
          if (tool ==  "text") {
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
                  std::scoped_lock lock{mutex};
                  command->RGBColour = colour;
                  command->item = text;
              }


          } else if (tool ==  "circle"){
              CircleShape circleShape;
              circleShape.x = std::stoi(tokens[1]);
              circleShape.y = std::stoi(tokens[2]);
              circleShape.radius = std::stof(tokens[3]);

              {
                  std::scoped_lock lock{mutex};
                  command->RGBColour = colour;
                  command->item = circleShape;
              }

          } else if (tool ==  "rectangle"){
              RectangleShape rectangleShape;
              rectangleShape.topLeftX = std::stoi(tokens[1]);
              rectangleShape.topLeftY = std::stoi(tokens[2]);
              rectangleShape.bottomRightX = std::stoi(tokens[3]);
              rectangleShape.bottomRightY = std::stoi(tokens[4]);

              {
                  std::scoped_lock lock{mutex};
                  command->RGBColour = colour;
                  command->item = rectangleShape;
              }
          } else if (tool ==  "line"){
              LineShape lineShape;
              lineShape.startX = std::stoi(tokens[1]);
              lineShape.endX = std::stoi(tokens[2]);
              lineShape.startY = std::stoi(tokens[3]);
              lineShape.endY = std::stoi(tokens[4]);

              {
                  std::scoped_lock lock{mutex};
                  command->RGBColour = colour;
                  command->item = lineShape;
              }
          }

        }
        else if(tokens[0] == "list"){
            // define if statements

            // continue processing
            std::cout << "Continue ..."<< std::endl;


        }

        else if(tokens[0] == "select"){
            int id;
            if(tokens[1] == "none"){
                // continue processing
                std::cout << "Continue ..."<< std::endl;

            } else{
                try{
                    id = std::stoi(tokens[1]);
                } catch(std::exception &err){
                    std::cout << "Invalid parameter for select!"<< std::endl;
                }

                // continue processing
                std::cout << "Continue ..."<< std::endl;

            }
        }

        else if (tokens[0] == "delete"){
            int id;
            try{
                id = std::stoi(tokens[1]);
            } catch(std::exception &err){
                std::cout << "Invalid parameter for delete!"<< std::endl;
            }

            // continue processing
            std::cout << "Continue ..." << std::endl;


        }
        else if (tokens[0] == "undo"){
            // continue processing
            std::cout << "Continue ..." << std::endl;
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
}

int main() {
    std::mutex mutex;
    SetTraceLogLevel(LOG_WARNING); // hide LOG messages from raylib
    Command command;
    std::thread inputThread(readInput, &command, std::ref(mutex));
    const int screenWidth = 1200;
    const int screenHeight = 750;
    InitWindow(screenWidth, screenHeight, "netsketch");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        {
            std::scoped_lock lock{mutex};

            Color color = {static_cast<unsigned char>(std::get<0>(command.RGBColour)),
                           static_cast<unsigned char>(std::get<1>(command.RGBColour)),
                           static_cast<unsigned char>(std::get<2>(command.RGBColour)), 255};
            if (std::holds_alternative<CircleShape>(command.item)) {
                auto &circleShape = std::get<CircleShape>(command.item);
                DrawCircle(circleShape.x, circleShape.y, circleShape.radius, color);
            }
            if (std::holds_alternative<RectangleShape>(command.item)) {
                auto &rectangleShape = std::get<RectangleShape>(command.item);
                int width = rectangleShape.bottomRightX - rectangleShape.topLeftX;
                int height = rectangleShape.bottomRightY - rectangleShape.topLeftY;
                int posX = (rectangleShape.topLeftX + rectangleShape.bottomRightX) / 2;
                int posY = (rectangleShape.topLeftY + rectangleShape.bottomRightY) / 2;
                //Color color = {static_cast<unsigned char>(std::get<0>(command.RGBColour)),static_cast<unsigned char>(std::get<1>(command.RGBColour)),static_cast<unsigned char>(std::get<2>(command.RGBColour)), 255 };
                DrawRectangle(posX, posY, width, height, color);
            }
            if (std::holds_alternative<LineShape>(command.item)) {
                auto &lineShape = std::get<LineShape>(command.item);
                //Color color = {static_cast<unsigned char>(std::get<0>(command.RGBColour)),static_cast<unsigned char>(std::get<1>(command.RGBColour)),static_cast<unsigned char>(std::get<2>(command.RGBColour)), 255 };
                DrawLine(lineShape.startX, lineShape.startY, lineShape.endX, lineShape.endY, color);
            }
            if (std::holds_alternative<TextShape>(command.item)) {
                auto &textShape = std::get<TextShape>(command.item);
                //Color color = {static_cast<unsigned char>(std::get<0>(command.RGBColour)),static_cast<unsigned char>(std::get<1>(command.RGBColour)),static_cast<unsigned char>(std::get<2>(command.RGBColour)), 255 };
                DrawText((textShape.text).c_str(), textShape.x, textShape.y, 30, color);
            }
        }

        EndDrawing();
    }

    inputThread.join();

    CloseWindow();

    return 0;
}