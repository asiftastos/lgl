#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "glad.h"
#include "GLFW/glfw3.h"
#include "struct.h"
#include "assets.h"

typedef enum RenderPass { PASS_3D, PASS_2D, PASS_UI, PASS_FLUSH }RenderPass;

typedef void(*DemoFunc)();

typedef struct Button
{
    bool down;
    bool last;
    bool pressed;
}Button;

typedef struct Mouse
{
    Button buttons[GLFW_MOUSE_BUTTON_LAST];
    vec2s position;
    vec2s delta;
    float wheelDelta;
}Mouse;

typedef struct Demo
{
    GLFWwindow* window;
    vec2s winSize;
    vec2s fbSize;
    DemoFunc init;
    DemoFunc terminate;
    DemoFunc update;
    DemoFunc render;
    int renderPass; //[PASS_3D or PASS_2D] used in render func, -1 for other funcs
    Button keys[GLFW_KEY_LAST];
    unsigned int lastChar;
    Mouse mouse;
    double frameDelta;
    double cpuTime;
}Demo;


Demo* demoCreate(DemoFunc init, DemoFunc terminate, DemoFunc update, DemoFunc render);
void demoDestroy();
void demoRun();