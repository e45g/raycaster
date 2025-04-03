#include <GLFW/glfw3.h>

void set_color(int r, int g, int b) {
    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;

    glColor3f(fr, fg, fb);
}
