#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#include "player.h"
#include "utils.h"

int move_forward = 0;
int move_backward = 0;
int rotate_left = 0;
int rotate_right = 0;

void handle_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    //warnings;
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_W && action == GLFW_PRESS) move_forward = 1;
    if (key == GLFW_KEY_S && action == GLFW_PRESS) move_backward = 1;

    if (key == GLFW_KEY_W && action == GLFW_RELEASE) move_forward = 0;
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) move_backward = 0;

    if (key == GLFW_KEY_A && action == GLFW_PRESS) rotate_left = 1;
    if (key == GLFW_KEY_D && action == GLFW_PRESS) rotate_right = 1;

    if (key == GLFW_KEY_A && action == GLFW_RELEASE) rotate_left = 0;
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) rotate_right = 0;
}

//
// void check_collision(int new_x, int new_y, player_t *player, int *map) {
//
// }
//

void handle_movement(player_t *player, int map[], float d_time) {
    // Store the accumulated sub-pixel movement
    static float x_remainder = 0;
    static float y_remainder = 0;

    // Handle rotation
    if (rotate_left) {
        player->angle -= ROTATION_SPEED * d_time;
        if(player->angle < 0) player->angle += 2*M_PI;
    }
    if (rotate_right) {
        player->angle += ROTATION_SPEED * d_time;
        if(player->angle > 2*M_PI) player->angle -= 2*M_PI;
    }

    // Update direction vectors based on current angle
    player->dx = cos(player->angle) * MOVE_SPEED;
    player->dy = sin(player->angle) * MOVE_SPEED;

    // Calculate movement with accumulated remainders
    float move_x = x_remainder;
    float move_y = y_remainder;

    if (move_forward) {
        move_x += player->dx * d_time;
        move_y += player->dy * d_time;
    }
    if (move_backward) {
        move_x -= player->dx * d_time;
        move_y -= player->dy * d_time;
    }

    // Apply integer movement and store remainder
    int int_move_x = (int)move_x;
    int int_move_y = (int)move_y;
    x_remainder = move_x - int_move_x;
    y_remainder = move_y - int_move_y;

    // Calculate new position
    int new_x = player->x + int_move_x;
    int new_y = player->y + int_move_y;

    // Perform collision detection
    // Check if the new position collides with a wall
    int block_x = new_x / BLOCKS_WIDTH;
    int block_y = new_y / BLOCKS_HEIGHT;
    int map_index = block_y * BLOCKS_X + block_x;

    // Assume non-zero values in map represent walls/obstacles
    if (map_index >= 0 && map_index < BLOCKS_X * BLOCKS_Y && map[map_index] == 0) {
        // No collision, update position
        player->x = new_x;
        player->y = new_y;
    } else {
        // Collision detected, try horizontal movement only
        map_index = player->y / BLOCKS_HEIGHT * BLOCKS_X + block_x;
        if (map_index >= 0 && map_index < BLOCKS_X * BLOCKS_Y && map[map_index] == 0) {
            player->x = new_x;
            // Since we didn't move vertically, clear the y remainder
            y_remainder = 0;
        } else {
            // Try vertical movement only
            map_index = block_y * BLOCKS_X + player->x / BLOCKS_WIDTH;
            if (map_index >= 0 && map_index < BLOCKS_X * BLOCKS_Y && map[map_index] == 0) {
                player->y = new_y;
                // Since we didn't move horizontally, clear the x remainder
                x_remainder = 0;
            } else {
                // Both directions have collision, don't move
                x_remainder = 0;
                y_remainder = 0;
            }
        }
    }

    // Additional check for player boundaries
    // Prevent player from going out of the map
    if (player->x < 0) {
        player->x = 0;
        x_remainder = 0;
    }
    if (player->y < 0) {
        player->y = 0;
        y_remainder = 0;
    }
    if (player->x >= BLOCKS_X * BLOCKS_WIDTH) {
        player->x = BLOCKS_X * BLOCKS_WIDTH - 1;
        x_remainder = 0;
    }
    if (player->y >= BLOCKS_Y * BLOCKS_HEIGHT) {
        player->y = BLOCKS_Y * BLOCKS_HEIGHT - 1;
        y_remainder = 0;
    }
}
