#include <raylib.h>
#include <unistd.h>

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 750;
    InitWindow(screenWidth, screenHeight, "test window");

    //while (!WindowShouldClose())
    //{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangleLines(100, 100, 200, 150 , RED );
    DrawText("Hello from user 1!", 190, 200, 20, LIGHTGRAY);
    DrawCircleLines( 100, 200, 50, GREEN);
    DrawLine(100, 50, 300, 400,BLUE);
    EndDrawing();
    sleep(4);
    //}

    CloseWindow();

    return 0;
}
