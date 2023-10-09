#include <stdio.h>
#include <math.h>
#include "/opt/homebrew/Cellar/raylib/4.5.0/include/raylib.h"
#include "/opt/homebrew/Cellar/raylib/4.5.0/include/raymath.h"
#include "textures.cpp"

// M_PI is a thing...
#define SCREEN_HEIGHT 800
#define SCREEN_WIDTH 1200
#define CUBE_WIDTH 25
#define BODY_RAD 15
#define raycount 1200
#define enemycount 64
#define ENEMY_RAD 5
#define TEXTURE_SIZE 25
#define TEXTURE_HEIGHT 12

#define Colour Color

float rotation = 0.0;
float visionCone = (PI * 0.6);
float stepSize = 0.01;
float viewGirth = 10;

enum GameState
{
    menu,
    game,
    deathscreen
};
enum ViewState
{
    bird,
    head
};
enum Status
{
    alive,
    dead
};

struct Laser
{
    float length;
    float angle;
    Vector2 edge;
    Vector2 start;
    Vector2 spotted;
    short spottedTrue;
    short x;
    short y;
};
struct BadGuy
{
    Vector2 location;
    float direction;
    Status status;
};

int main(void)
{

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycaster");
    SetWindowPosition(60, 60);

    GameState gamestate = game;

    // Charcter
    Vector2 pos = (Vector2){300, 300};
    Vector2 cam = (Vector2){0, 0};
    Vector2 mov = (Vector2){0, 0};
    int vol = 200;
    ViewState view = bird;

    // Rays
    Laser cast[raycount];

    // Enemies
    BadGuy enemies[enemycount];
    int enemyVol = 50;
    for (int e = 0; e < enemycount; e++)
    {
        enemies[e].direction = 0.0f;
        enemies[e].status = dead;
    }

    // Enemy test
    enemies[7].location = (Vector2){200, 700};
    enemies[7].status = alive;

    // enemies[8].location = (Vector2){1000, 200};
    // enemies[8].status = alive;

    // 24 across, 16 high
    int arr[48][32];

    for (int i = 0; i < 48; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (i == 0 || i == 47)
            {
                arr[i][j] = 1;
            }
            else if (j == 0 || j == 31)
            {
                arr[i][j] = 1;
            }
            else
            {
                arr[i][j] = 0;
            }
        }
    }
    // Random Objects
    arr[5][5] = 1;
    arr[5][6] = 1;
    arr[5][7] = 1;
    arr[6][7] = 1;
    arr[7][7] = 1;
    arr[12][8] = 1;
    arr[12][9] = 1;
    arr[13][8] = 1;
    arr[13][9] = 1;

    while (!WindowShouldClose())
    {
        switch (gamestate)
        {
        case menu:
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                enemies[7].location = (Vector2){200, 700};
                gamestate = game;
            }

            BeginDrawing();

            ClearBackground(WHITE);

            DrawText("ShootyShooty Game",
                     450,
                     300,
                     60,
                     DARKGRAY);

            EndDrawing();
        }
        break;
        case game:
        {
// Input
#pragma region input
            // ------------------------------------
            mov.x = 0;
            mov.y = 0;

            if (IsKeyDown(KEY_UP))
            {
                mov.y = vol * sinf(rotation);
                mov.x = vol * cosf(rotation);
            }
            else if (IsKeyDown(KEY_DOWN))
            {
                mov.y = -1 * vol * sinf(rotation);
                mov.x = -1 * vol * cosf(rotation);
            }
            else if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_A))
            {
                mov.y = vol * sin(rotation + (PI * 0.5));
                mov.x = vol * cos(rotation + (PI * 0.5));
            }
            else if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_D))
            {
                mov.y = vol * sinf(rotation - (PI * 0.5));
                mov.x = vol * cosf(rotation - (PI * 0.5));
            }
            if (IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT))
            {
                rotation -= 2 * GetFrameTime();
                if (rotation < 0)
                {
                    rotation += 2 * PI;
                }
            }
            if (IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT))
            {
                rotation += 2 * GetFrameTime();
                if (rotation > 2 * PI)
                {
                    rotation -= 2 * PI;
                }
            }

            if (IsKeyDown(KEY_B))
            {
                view = bird;
            }
            else if (IsKeyDown(KEY_H))
            {
                view = head;
            }
            // ------------------------------------
#pragma endregion

            // Update
#pragma region update
            // ------------------------------------

            // Player movement
            mov.x = (mov.x * GetFrameTime());
            mov.y = (mov.y * GetFrameTime());

            pos.x += mov.x;
            pos.y += mov.y;

            // Player Map Collision (I am well aware that this is an abomination, but it works... for now)
            for (int i = 0; i < 48; i++)
            {
                for (int j = 0; j < 32; j++)
                {
                    if (arr[i][j] == 1)
                    {
                        if (pos.y > j * CUBE_WIDTH - BODY_RAD && pos.y < j * CUBE_WIDTH + CUBE_WIDTH + BODY_RAD)
                        {
                            if (pos.x > i * CUBE_WIDTH - BODY_RAD && pos.x < i * CUBE_WIDTH + CUBE_WIDTH + BODY_RAD)
                            {
                                pos.x -= mov.x;
                            }
                        }
                        if (pos.x > i * CUBE_WIDTH - BODY_RAD && pos.x < i * CUBE_WIDTH + CUBE_WIDTH + BODY_RAD)
                        {
                            if (pos.y > j * CUBE_WIDTH - BODY_RAD && pos.y < j * CUBE_WIDTH + CUBE_WIDTH + BODY_RAD)
                            {
                                pos.y -= mov.y;
                            }
                        }
                    }
                }
            }

            // Enemy movement
            for (int e = 0; e < enemycount; e++)
            {
                if (alive == enemies[e].status)
                {
                    enemies[e].direction = Vector2Angle(enemies[e].location, pos);
                    enemies[e].location.y += enemyVol * sinf(enemies[e].direction) * GetFrameTime();
                    enemies[e].location.x += enemyVol * cosf(enemies[e].direction) * GetFrameTime();
                }
            }

            // Reset enemy position map
            for (int i = 0; i < 48; i++)
            {
                for (int j = 0; j < 32; j++)
                {
                    if (2 == arr[i][j])
                    {
                        arr[i][j] = 0;
                    }
                }
            }

            // Enemy collision
            short chunkX = 0;
            short chunkY = 0;
            for (int e = 0; e < enemycount; e++)
            {
                if (alive == enemies[e].status)
                {
                    chunkX = (int)(enemies[e].location.x / CUBE_WIDTH);
                    chunkY = (int)(enemies[e].location.y / CUBE_WIDTH);
                    if (Vector2Distance(pos, enemies[e].location) < BODY_RAD + ENEMY_RAD)
                    {
                        gamestate = deathscreen;
                    }
                    if (1 != arr[chunkX][chunkY])
                    {
                        arr[chunkX][chunkY] = 2;
                    }

                    // Above -
                    if (0 >= enemies[e].direction)
                    {
                        if (1 != arr[chunkX][chunkY - 1])
                        {
                            arr[chunkX][chunkY - 1] = 2;
                        }

                        // Right -
                        if (PI * 0.5 * (-1) <= enemies[e].direction)
                        {
                            if (1 != arr[chunkX + 1][chunkY])
                            {
                                arr[chunkX + 1][chunkY] = 2;
                            }
                            if (1 != arr[chunkX + 1][chunkY - 1])
                            {
                                arr[chunkX + 1][chunkY - 1] = 2;
                            }
                        }
                        // Left -
                        else
                        {
                            if (1 != arr[chunkX - 1][chunkY])
                            {
                                arr[chunkX - 1][chunkY] = 2;
                            }
                            if (1 != arr[chunkX - 1][chunkY - 1])
                            {
                                arr[chunkX - 1][chunkY - 1] = 2;
                            }
                        }
                    }
                    // Below -
                    else
                    {
                        if (1 != arr[chunkX][chunkY + 1])
                        {
                            arr[chunkX][chunkY + 1] = 2;
                        }

                        // Left -
                        if (PI * 0.5 <= enemies[e].direction)
                        {
                            if (1 != arr[chunkX - 1][chunkY])
                            {
                                arr[chunkX - 1][chunkY] = 2;
                            }
                            if (1 != arr[chunkX - 1][chunkY + 1])
                            {
                                arr[chunkX - 1][chunkY + 1] = 2;
                            }
                        }
                        // Right -
                        else
                        {
                            if (1 != arr[chunkX + 1][chunkY])
                            {
                                arr[chunkX + 1][chunkY] = 2;
                            }
                            if (1 != arr[chunkX + 1][chunkY + 1])
                            {
                                arr[chunkX + 1][chunkY + 1] = 2;
                            }
                        }
                    }
                }
            }

            cam.x = pos.x + 30 * cosf(rotation);
            cam.y = pos.y + 30 * sinf(rotation);

            Vector2 bearth = Vector2Zero();

            for (int k = 0; k < raycount; k++)
            {
                bearth = Vector2Rotate((Vector2){0, (float)(-1 * viewGirth + (k * (viewGirth / (raycount * 0.5))))},
                                       rotation);

                cast[k].start.x = pos.x + bearth.x;
                cast[k].edge.x = cast[k].start.x;
                cast[k].start.y = pos.y + bearth.y;
                cast[k].edge.y = cast[k].start.y;
            }

            float angleDiff;
            // Raycast collisions
            for (int k = 0; k < raycount; k++)
            {
                short hit = 0;

                cast[k].spottedTrue = 0;

                cast[k].angle = rotation -
                                ((visionCone * 0.5) -
                                 (visionCone / (2 * raycount))) +
                                ((visionCone / raycount) * k);

                while (true)
                {
                    cast[k].x = (int)(cast[k].edge.x / CUBE_WIDTH);
                    cast[k].y = (int)(cast[k].edge.y / CUBE_WIDTH);
                    if (arr[cast[k].x][cast[k].y] == 1)
                    {
                        hit = 1;
                        break;
                    }

                    // Enemy Spotted!
                    if (arr[cast[k].x][cast[k].y] == 2)
                    {
                        for (int e = 0; e < enemycount; e++)
                        {
                            if (ENEMY_RAD > Vector2Distance(cast[k].edge, enemies[e].location))
                            {
                                cast[k].spotted.x = cast[k].edge.x;
                                cast[k].spotted.y = cast[k].edge.y;
                                cast[k].spottedTrue = 1;
                            }
                        }
                    }

                    if (hit == 1)
                    {
                        break;
                    }

                    if (0 < arr[cast[k].x + 1][cast[k].y] ||
                        0 < arr[cast[k].x - 1][cast[k].y] ||
                        0 < arr[cast[k].x][cast[k].y + 1] ||
                        0 < arr[cast[k].x][cast[k].y - 1] ||
                        0 < arr[cast[k].x + 1][cast[k].y + 1] ||
                        0 < arr[cast[k].x + 1][cast[k].y - 1] ||
                        0 < arr[cast[k].x - 1][cast[k].y + 1] ||
                        0 < arr[cast[k].x - 1][cast[k].y - 1])
                    {
                        cast[k].edge.x += cosf(cast[k].angle) * CUBE_WIDTH * stepSize;
                        cast[k].edge.y += sinf(cast[k].angle) * CUBE_WIDTH * stepSize;
                    }
                    else
                    {
                        cast[k].edge.x += cosf(cast[k].angle) * CUBE_WIDTH;
                        cast[k].edge.y += sinf(cast[k].angle) * CUBE_WIDTH;
                    }
                }

                // Fisheye Correction
                angleDiff = rotation - cast[k].angle;
                if (angleDiff < 0)
                {
                    angleDiff += PI * 2;
                }
                else if (angleDiff > 2 * PI)
                {
                    angleDiff -= PI * 2;
                }
                cast[k].length = cosf(angleDiff) * Vector2Distance(pos, cast[k].edge);
            }

            // ------------------------------------
#pragma endregion

            // Draw
            // ------------------------------------
            BeginDrawing();

            switch (view)
            {
                // Top-Down Perspective
            case bird:
            {
                ClearBackground(LIGHTGRAY);
                // Objects / Walls
                for (int i = 0; i < 48; i++)
                {
                    for (int j = 0; j < 32; j++)
                    {
                        if (arr[i][j] == 1)
                        {
                            DrawRectangle(
                                (i * CUBE_WIDTH),
                                (j * CUBE_WIDTH),
                                CUBE_WIDTH,
                                CUBE_WIDTH,
                                DARKGRAY);
                        }
                        else if (arr[i][j] == 2)
                        {
                            DrawRectangle(
                                (i * CUBE_WIDTH),
                                (j * CUBE_WIDTH),
                                CUBE_WIDTH,
                                CUBE_WIDTH,
                                GREEN);
                        }
                    }
                }
                // Player & Rays
                DrawCircleV(pos, 20.0, BLACK);
                for (int k = 0; k < raycount; k++)
                {
                    DrawLineEx(cast[k].start, cast[k].edge, 2.0, RED);
                }
                DrawLineEx(pos, cam, 3.0, BLACK);

                // Enemies
                for (int e = 0; e < enemycount; e++)
                {
                    if (alive == enemies[e].status)
                    {
                        DrawCircleV(enemies[e].location, 10.0, BLUE);
                    }
                }
            }
            break;

                // Player's Perspective
            case head:
            {
                ClearBackground(BLACK);

                float wallTop;
                float wallBottom;
                float height = 0.0;
                float enemyHeight = 0.0;

                Colour colour = RED;

                int wallX = 0;
                int wallY = 0;
                int wallRelevant = 0;

                for (int k = 0; k < raycount; k++)
                {
                    // Calculating distance and height of walls
                    height = 10000.0 / cast[k].length;
                    wallTop = SCREEN_HEIGHT * 0.5 - height;
                    wallBottom = SCREEN_HEIGHT * 0.5 + height;
                    if (1 == cast[k].spottedTrue)
                    {
                        enemyHeight = 8000.0 / Vector2Distance(cast[k].start, cast[k].spotted);
                    }
                    else
                    {
                        enemyHeight = 0.0;
                    }

                    // Checks distance on block
                    wallX = (int)cast[k].edge.x % CUBE_WIDTH;
                    wallY = (int)cast[k].edge.y % CUBE_WIDTH;

                    // Uses "Distance on block" to give the correct texture
                    if (wallX == 0 || wallX == TEXTURE_SIZE - 1)
                    {
                        wallRelevant = wallY;
                    }
                    else
                    {
                        wallRelevant = wallX;
                    }

                    // Draw player view: first sky, then wall, then floor.
                    DrawLine(k, 0, k, wallTop, LIGHTGRAY);

                    if (height > enemyHeight)
                    {
                        for (int d = 0; d < TEXTURE_HEIGHT; d++)
                        {
                            colour = (Colour){0,
                                              (unsigned char)(255 - 100 * wallTexture[wallRelevant + TEXTURE_SIZE * d]),
                                              (unsigned char)(255 - 50 * wallTexture[wallRelevant + TEXTURE_SIZE * d]),
                                              255};

                            DrawLine(k, wallTop + (height * 2 / TEXTURE_HEIGHT) * d,
                                     k, wallTop + (height * 2 / TEXTURE_HEIGHT) * (d + 1),
                                     colour);
                        }
                    }

                    DrawLine(k, wallBottom, k, SCREEN_HEIGHT, (Colour){40, 15, 40, 255});

                    if (1 == cast[k].spottedTrue)
                    {
                        DrawRectangle(k - 1, (SCREEN_HEIGHT / 2) - enemyHeight, 3, enemyHeight * 2, WHITE);
                        // DrawRectangle(k - 1, (SCREEN_HEIGHT / 2) - enemyHeight, 3, enemyHeight * 2, RED);
                    }
                }
            }
            break;
            }
            // Useful Data
            DrawText(TextFormat("FPS: %03i", GetFPS()),
                     SCREEN_WIDTH - 200,
                     SCREEN_HEIGHT - 50,
                     20,
                     DARKGRAY);
            DrawText(TextFormat("Rotation: %04f", rotation),
                     SCREEN_WIDTH - 200,
                     SCREEN_HEIGHT - 70,
                     20,
                     DARKGRAY);

            EndDrawing();
            // ------------------------------------
        }
        break;
        case deathscreen:
        {

            if (IsKeyPressed(KEY_SPACE))
            {
                gamestate = menu;
            }

            BeginDrawing();
            ClearBackground(WHITE);
            DrawText("You're hella' dead.",
                     SCREEN_WIDTH / 2 - 200,
                     SCREEN_HEIGHT - 500,
                     40,
                     BLACK);
            DrawText("Wanna try again? [SPACE]",
                     SCREEN_WIDTH / 2 - 200,
                     SCREEN_HEIGHT - 400,
                     40,
                     BLACK);
            DrawText(TextFormat("FPS: %03i", GetFPS()),
                     SCREEN_WIDTH - 200,
                     SCREEN_HEIGHT - 50,
                     20,
                     DARKGRAY);
            EndDrawing();
        }
        break;
        }
    }

    return 0;
}
