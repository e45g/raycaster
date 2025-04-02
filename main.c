#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>

#define MIN(a, b) (a>b)?b:a

#define WINDOW_WIDTH 1800
#define WINDOW_HEIGHT 1200

#define MINIMAP_WIDTH 200
#define MINIMAP_HEIGHT 200

#define COLLISION_BUFFER 5

typedef struct {
    int x;
    int y;
    int w;
    int h;
    float angle;
    float dx;
    float dy;
} player_t;

void draw_borders(SDL_Renderer *renderer, int map_blocks) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for(int i = 0; i < map_blocks; i++) {
        SDL_RenderDrawLine(renderer, i * MINIMAP_WIDTH/map_blocks, 0, i * MINIMAP_WIDTH/map_blocks, MINIMAP_HEIGHT);
    }

    for(int i = 0; i < map_blocks; i++) {
        SDL_RenderDrawLine(renderer, 0, i*MINIMAP_HEIGHT/map_blocks, MINIMAP_WIDTH, i*MINIMAP_HEIGHT/map_blocks);
    }
}


void draw_obstacles(SDL_Renderer *renderer, const char map[], int map_blocks) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for(int i = 0; i < map_blocks; i++) {
        for(int j = 0; j < map_blocks; j++) {
            if(map[i+map_blocks*j] == '#') {
                SDL_Rect rect = {
                    .y = j * MINIMAP_HEIGHT / map_blocks,
                    .x = i * MINIMAP_WIDTH / map_blocks,
                    .w = MINIMAP_WIDTH / map_blocks,
                    .h = MINIMAP_HEIGHT / map_blocks
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

int collides_with_wall(player_t *player, int map_blocks, const char map[]) {
    float cell_width = MINIMAP_WIDTH / (float)map_blocks;
    float cell_height = MINIMAP_HEIGHT / (float)map_blocks;

    float left   = player->x - player->w / 2.0f;
    float right  = player->x + player->w / 2.0f;
    float top    = player->y - player->h / 2.0f;
    float bottom = player->y + player->h / 2.0f;

    int corners[4][2] = {
        {(int)(left   / cell_width), (int)(top    / cell_height)},
        {(int)(right  / cell_width), (int)(top    / cell_height)},
        {(int)(left   / cell_width), (int)(bottom / cell_height)},
        {(int)(right  / cell_width), (int)(bottom / cell_height)}
    };

    for (int i = 0; i < 4; i++) {
        int cx = corners[i][0];
        int cy = corners[i][1];
        if (cx < 0 || cx >= map_blocks || cy < 0 || cy >= map_blocks)
            return 1;
        if (map[cy * map_blocks + cx] == '#')
            return 1;
    }
    return 0;
}

void move_player(player_t *player, float move_step, int map_blocks, const char map[]) {
    float old_x = player->x;
    float old_y = player->y;

    player->x += player->dx * move_step;
    if (collides_with_wall(player, map_blocks, map)) {
        player->x = old_x;
    }

    player->y += player->dy * move_step;
    if (collides_with_wall(player, map_blocks, map)) {
        player->y = old_y;
    }
}

void draw_player(SDL_Renderer *renderer, player_t *player) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect pl = {(int)(player->x - player->w / 2), (int)(player->y - player->h / 2), player->w, player->h};
    SDL_RenderFillRect(renderer, &pl);
    SDL_RenderDrawLine(renderer, (int)player->x, (int)player->y, (int)(player->x + player->dx * 5), (int)(player->y + player->dy * 5));
}


void draw_rays(SDL_Renderer *renderer, player_t *player, int map_blocks, const char map[]) {

    int num_rays = 60;
    float fov = 60 * (M_PI / 180);
    float start_angle = player->angle - (fov / 2);

    float cell_width = MINIMAP_WIDTH / (float)map_blocks;
    float cell_height = MINIMAP_HEIGHT / (float)map_blocks;

    int strip_width = WINDOW_WIDTH / num_rays;

    // draw sky
    SDL_SetRenderDrawColor(renderer, 50, 50, 120, 255);
    SDL_Rect sky = {
        .x = 0,
        .y = 0,
        .w = WINDOW_WIDTH,
        .h = WINDOW_HEIGHT / 2
    };
    SDL_RenderFillRect(renderer, &sky);

    //draw floor
    SDL_SetRenderDrawColor(renderer, 120, 120, 50, 255);
    SDL_Rect flr = {
        .x = 0,
        .y = WINDOW_HEIGHT / 2,
        .w = WINDOW_WIDTH,
        .h = WINDOW_HEIGHT
    };
    SDL_RenderFillRect(renderer, &flr);

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

        float delta_dist_x = fabs(cell_width / (rd_x != 0 ? rd_x : 0.000001));
        float delta_dist_y = fabs(cell_height / (rd_y != 0 ? rd_y : 0.000001));

        float ray_length_x = (next_x - player->x) / (rd_x != 0 ? rd_x : 0.000001);
        float ray_length_y = (next_y - player->y) / (rd_y != 0 ? rd_y : 0.000001);

        if (rd_x < 0) ray_length_x = (player->x - map_x * cell_width) / fabs(rd_x);
        if (rd_y < 0) ray_length_y = (player->y - map_y * cell_height) / fabs(rd_y);

        int hit = 0;
        int hit_side = 0;
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

            if(map_x < 0 || map_x >= map_blocks || map_y < 0 || map_y >= map_blocks) break;

            if(map[map_y * map_blocks + map_x] == '#') hit = 1;
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer,
                           player->x,
                           player->y,
                           player->x + rd_x * ray_distance,
                           player->y + rd_y * ray_distance
                           );


        // Walls
        float angle_diff = ray_angle - player->angle;
        if (angle_diff < 0) angle_diff += 2 * M_PI;
        if (angle_diff > 2 * M_PI) angle_diff -= 2 * M_PI;
        float corrected_distance = ray_distance * cos(angle_diff);

        int line_height = (int)(WINDOW_HEIGHT / corrected_distance * 50);
        if (line_height > WINDOW_HEIGHT) line_height = WINDOW_HEIGHT;

        int line_start = (WINDOW_HEIGHT - line_height) / 2;

        SDL_Rect wall_strip = {
            i * strip_width,
            line_start,
            strip_width,
            line_height
        };

        if(hit_side == 1){
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        } else{
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        }

        SDL_RenderFillRect(renderer, &wall_strip);
    }
}



int main(void) {
    player_t player = {
        .x = MINIMAP_WIDTH / 2,
        .y = MINIMAP_HEIGHT / 2,
        .w = MINIMAP_WIDTH / 25,
        .h = MINIMAP_HEIGHT / 25,
        .angle = 0,
        .dx = 0,
        .dy = 0
    };

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    const char map[] = {
        '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#',
        '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
        '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
    };
    int map_len = sizeof(map);
    int map_blocks = sqrt(map_len);

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not be initialized!\nSDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("hehe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if(!window)
    {
        printf("Window could not be created!\nSDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!renderer)
    {
        printf("Renderer could not be created!\nSDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    int quit = 0;
    while(!quit) {
        SDL_Event e;
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) quit = 1;

        const Uint8* kb = SDL_GetKeyboardState(NULL);
        if (kb[SDL_SCANCODE_W]) move_player(&player, 2.0f, map_blocks, map);
        if (kb[SDL_SCANCODE_S]) move_player(&player, -2.0f, map_blocks, map);
        if (kb[SDL_SCANCODE_A]) player.angle -= 0.05f;
        if (kb[SDL_SCANCODE_D]) player.angle += 0.05f;

        player.dx = cos(player.angle) * 1.0f;
        player.dy = sin(player.angle) * 1.0f;

        SDL_RenderClear(renderer);

        draw_rays(renderer, &player, map_blocks, map);
        draw_obstacles(renderer, map, map_blocks);
        draw_player(renderer, &player);
        draw_borders(renderer, map_blocks);


        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
