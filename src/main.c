#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "player.h"
#include "utils.h"
#include "textures.h"

void draw_obstacles(int *map) {
    set_color(200, 50, 50);

    for(int y = 0; y < BLOCKS_Y; y++) {
        for(int x = 0; x < BLOCKS_X; x++) {
            if(map[y*BLOCKS_X+x] < 1) continue;
            glBegin(GL_QUADS);

            glVertex2i(x * BLOCKS_WIDTH, y * BLOCKS_HEIGHT);
            glVertex2i((x + 1) * BLOCKS_WIDTH, y * BLOCKS_HEIGHT);
            glVertex2i((x + 1) * BLOCKS_WIDTH, (y + 1) * BLOCKS_HEIGHT);
            glVertex2i(x * BLOCKS_WIDTH, (y + 1) * BLOCKS_HEIGHT);

            glEnd();
        }
    }
}

void draw_map() {
    set_color(200, 200, 200);

    for(int i = 0; i < BLOCKS_Y; i++) {
        glBegin(GL_LINES);
        glVertex2i(0, i * BLOCKS_HEIGHT);
        glVertex2i(MINIMAP_WIDTH, i * BLOCKS_HEIGHT);
        glEnd();
    }

    for(int i = 0; i < BLOCKS_X; i++) {
        glBegin(GL_LINES);
        glVertex2i(i * BLOCKS_WIDTH, 0);
        glVertex2i(i * BLOCKS_WIDTH, MINIMAP_HEIGHT);
        glEnd();
    }

}

void draw_player(player_t *player) {
    glColor3f(0, 0.8, 0);
    float left = player->x - player->w / 2.0f;
    float right = player->x + player->w / 2.0f;
    float top = player->y - player->h / 2.0f;
    float bottom = player->y + player->h / 2.0f;

    glBegin(GL_QUADS);
    glVertex2f(left, top);
    glVertex2f(right, top);
    glVertex2f(right, bottom);
    glVertex2f(left, bottom);

    glEnd();
}

void draw_rays(player_t *player, int *map_walls, int *map_floor) {
    int num_rays = 120;
    float fov = 60 * (M_PI / 180);
    float start_angle = player->angle - (fov / 2);

    float cell_width = MINIMAP_WIDTH / (float)BLOCKS_X;
    float cell_height = MINIMAP_HEIGHT / (float)BLOCKS_Y;

    int strip_width = WINDOW_WIDTH / num_rays;

    set_color(20, 50, 90);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(WINDOW_WIDTH, 0);
    glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT/2);
    glVertex2i(0, WINDOW_HEIGHT/2);
    glEnd();

    for(int i = 0; i < num_rays; i++) {
        float ray_angle = start_angle + (i / (float)num_rays) * fov;

        if(ray_angle < 0) ray_angle += 2 * M_PI;
        if(ray_angle > 2 * M_PI) ray_angle -= 2 * M_PI;

        float rd_x = cos(ray_angle);
        float rd_y = sin(ray_angle);

        int map_x = player->x / cell_width;
        int map_y = player->y / cell_height;

        int step_x = (rd_x < 0) ? -1 : 1;
        int step_y = (rd_y < 0) ? -1 : 1;

        float next_x = (rd_x > 0) ? (map_x + 1) * cell_width : map_x * cell_width;
        float next_y = (rd_y > 0) ? (map_y + 1) * cell_height : map_y * cell_height;

        float delta_dist_x = fabs(cell_width / (fabs(rd_x) > 0.000001 ? rd_x : 0.000001));
        float delta_dist_y = fabs(cell_height / (fabs(rd_y) > 0.000001 ? rd_y : 0.000001));

        float ray_length_x = (next_x - player->x) / (rd_x != 0 ? rd_x : 0.000001);
        float ray_length_y = (next_y - player->y) / (rd_y != 0 ? rd_y : 0.000001);

        if (rd_x < 0) ray_length_x = (player->x - map_x * cell_width) / fabs(rd_x);
        if (rd_y < 0) ray_length_y = (player->y - map_y * cell_height) / fabs(rd_y);

        int hit = 0;
        int hit_side = 0;
        int wall_type = -1;
        float ray_distance = 0;

        while(!hit) {
            if(ray_length_x < ray_length_y) {
                map_x += step_x;
                ray_distance = ray_length_x;
                ray_length_x += delta_dist_x;
                hit_side = 0;
            }
            else {
                map_y += step_y;
                ray_distance = ray_length_y;
                ray_length_y += delta_dist_y;
                hit_side = 1;
            }

            if(map_x < 0 || map_x >= BLOCKS_X || map_y < 0 || map_y >= BLOCKS_Y) break;

            wall_type = map_walls[map_y * BLOCKS_X + map_x];
            if(wall_type > 0) hit = 1;
        }

        set_color(50, 100, 100);
        glBegin(GL_LINES);
        glVertex2f(player->x, player->y);
        glVertex2f(player->x + rd_x * ray_distance, player->y + rd_y * ray_distance);
        glEnd();

        float angle_diff = ray_angle - player->angle;
        if (angle_diff < 0) angle_diff += 2 * M_PI;
        if (angle_diff > 2 * M_PI) angle_diff -= 2 * M_PI;
        float corrected_distance = ray_distance * cos(angle_diff);
        if (corrected_distance < 0.1f) corrected_distance = 0.1f;

        int line_height = (int)(WINDOW_HEIGHT / corrected_distance * 25);
        if (line_height > WINDOW_HEIGHT * 3) line_height = WINDOW_HEIGHT * 3;
        int line_start = (WINDOW_HEIGHT - line_height) / 2;

        int x_start = i * strip_width;
        float shade = (hit_side == 0) ? 1.0f : 0.7f;

        int texture_x;
        if (hit_side == 0) {
            float hit_y = player->y + ray_distance * rd_y;
            float wall_hit = fmod(hit_y, cell_height) / cell_height;
            texture_x = (int)(wall_hit * 32);
            if (rd_x < 0) texture_x = 31 - texture_x;
        } else {
            float hit_x = player->x + ray_distance * rd_x;
            float wall_hit = fmod(hit_x, cell_width) / cell_width;
            texture_x = (int)(wall_hit * 32);
            if (rd_y > 0) texture_x = 31 - texture_x;
        }


        glPointSize(strip_width);
        glBegin(GL_POINTS);
        for (int y = line_start; y < line_start+line_height; y += strip_width) {
            int d = y - line_start;
            int texture_y = ((d * 32) / line_height) % 32;
            int index = (texture_y * 32 + texture_x) * 3 + (wall_type-1) * 32 * 32 * 3;
            int r = all_textures[index] * shade;
            int g = all_textures[index + 1] * shade;
            int b = all_textures[index + 2] * shade;
            glColor3ub(r, g, b);
            glVertex2i(x_start + strip_width / 2, y);
        }
        glEnd();
        glPointSize(1.0f);


        // draw floor

    }
}



int main(void) {
    player_t player = {
        .x = MINIMAP_WIDTH / 2,
        .y = MINIMAP_HEIGHT / 2,
        .w = MINIMAP_WIDTH / 25,
        .h = MINIMAP_HEIGHT / 25,
        .angle = M_PI,
        .dx = 0,
        .dy = 0
    };

    int map_walls[BLOCKS_Y * BLOCKS_X] = {0};
    for(int i = 0; i < BLOCKS_Y; i++) {
        for(int j = 0; j < BLOCKS_X; j++) {
            if(i == 0 || j == 0 || j == BLOCKS_X-1 || i == BLOCKS_Y - 1) {
                map_walls[i*BLOCKS_X+j] = (i*BLOCKS_X+j) % 20 + 1;
            }
        }
    }
    int map_floor[BLOCKS_Y * BLOCKS_X] = {0};
    for(int i = 0; i < BLOCKS_Y; i++) {
        for(int j = 0; j < BLOCKS_X; j++) {
            if(i == 0 || j == 0 || j == BLOCKS_X-1 || i == BLOCKS_Y - 1) {
                map_floor[i*BLOCKS_X+j] = 1;
            }
        }
    }


    if(!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hehe", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);

    // Set V-Sync (0 = uncapped FPS, 1 = sync to refresh rate)
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, handle_keyboard);

    float last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        float d_time = current_time - last_time;
        last_time = current_time;

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        draw_rays(&player, map_walls, map_floor);
        draw_obstacles(map_walls);
        draw_player(&player);
        draw_map();

        handle_movement(&player, map_walls, d_time);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
