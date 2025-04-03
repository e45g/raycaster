#pragma once

#define MIN(a, b) (a>b)?b:a

#define WINDOW_WIDTH 1800
#define WINDOW_HEIGHT 1200

#define BLOCKS_X 8
#define BLOCKS_Y 8

#define MINIMAP_WIDTH BLOCKS_X * 30
#define MINIMAP_HEIGHT BLOCKS_Y * 30

#define BLOCKS_WIDTH (MINIMAP_WIDTH/BLOCKS_X)
#define BLOCKS_HEIGHT (MINIMAP_HEIGHT/BLOCKS_Y)

void set_color(int r, int g, int b);
