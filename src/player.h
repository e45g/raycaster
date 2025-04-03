#pragma once

#include <GLFW/glfw3.h>
#define MOVE_SPEED 150.0f
#define ROTATION_SPEED 2.5f
#define COLLISION_BUFFER 00.0f

typedef struct {
    int x;
    int y;
    int w;
    int h;
    float angle;
    float dx;
    float dy;
} player_t;

void handle_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
void handle_movement(player_t *player, int map[], float d_time);
