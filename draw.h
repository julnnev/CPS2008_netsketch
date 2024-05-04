#ifndef NETSKETCH_DRAW_H
#define NETSKETCH_DRAW_H

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

struct Draw {
    Item item;
    std::tuple<int, int, int> RGBColour;
};

#endif //NETSKETCH_DRAW_H