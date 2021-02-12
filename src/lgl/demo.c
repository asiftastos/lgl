#include "demo.h"

static Demo* demo = NULL;

static void demoErrorCallback(int error, const char* desc)
{
    _CRT_UNUSED(error);
    printf("GLFW error: %s\n", desc);
}

static void demoFramebufferSize(GLFWwindow* win, int w, int h)
{
    _CRT_UNUSED(win);
    demo->fbSize = (vec2s){{w, h}};
    glViewport(0, 0, w, h);
}

static void demoWindowSize(GLFWwindow* win, int w, int h)
{
    _CRT_UNUSED(win);
    demo->winSize = (vec2s){{w, h}};
}

static void demoCursorPos(GLFWwindow* win, double x, double y)
{
    _CRT_UNUSED(win);
    demo->mouse.delta = glms_vec2_sub((vec2s){{x, y}}, demo->mouse.position);
    demo->mouse.position = (vec2s){{x,y}};
}

static void demoKey(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    _CRT_UNUSED(win);
    _CRT_UNUSED(scancode);
    _CRT_UNUSED(mods);

    if(key < 0) return;

    switch (action)
    {
    case GLFW_PRESS:
        demo->keys[key].down = true;
        break;
    case GLFW_RELEASE:
        demo->keys[key].down = false;
        break;
    default:
        break;
    }
}

static void demoChar(GLFWwindow* win, unsigned int codepoint)
{
    _CRT_UNUSED(win);
    demo->lastChar = codepoint;
}

static void demoMouseButton(GLFWwindow* win, int button, int action, int mods)
{
    _CRT_UNUSED(win);
    _CRT_UNUSED(mods);

    if(button < 0) return;

    switch (action)
    {
    case GLFW_PRESS:
        demo->mouse.buttons[button].down = true;
        break;
    case GLFW_RELEASE:
        demo->mouse.buttons[button].down = false;
        break;
    default:
        break;
    }
}

static void demoScroll(GLFWwindow* win, double xoff, double yoff)
{
    _CRT_UNUSED(win);

    demo->mouse.wheelDelta = yoff;
}

Demo* demoCreate(DemoFunc init, DemoFunc terminate, DemoFunc update, DemoFunc render)
{
    demo = (Demo*)calloc(1, sizeof(Demo));
    assert(demo);

    demo->winSize = (vec2s){{800, 600}};
    demo->init = init;
    demo->terminate = terminate;
    demo->update = update;
    demo->render = render;
    demo->renderPass = -1;
    demo->lastChar = 0;

    //glfw init
    glfwSetErrorCallback(demoErrorCallback);
    int r = glfwInit();
    assert(r == GLFW_TRUE);
    printf("GLFW: %s\n", glfwGetVersionString());

    //window init
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    demo->window = glfwCreateWindow(demo->winSize.x, demo->winSize.y, "Demo", NULL, NULL);
    assert(demo->window);
    const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int xpos = (vmode->width - demo->winSize.x) / 2;
    int ypos = (vmode->height - demo->winSize.y) / 2;
    glfwSetWindowPos(demo->window, xpos, ypos);
    glfwShowWindow(demo->window);
    glfwMakeContextCurrent(demo->window);

    //opengl init
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    printf("GL: %s\n", glGetString(GL_VERSION));
    printf("GL: %s\n", glGetString(GL_RENDERER));
    printf("GL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);

    glfwSwapInterval(1);

    //glfw callbacks
    glfwSetFramebufferSizeCallback(demo->window, demoFramebufferSize);
    glfwSetWindowSizeCallback(demo->window, demoWindowSize);
    glfwSetCursorPosCallback(demo->window, demoCursorPos);
    glfwSetKeyCallback(demo->window, demoKey);
    glfwSetMouseButtonCallback(demo->window, demoMouseButton);
    glfwSetCharCallback(demo->window, demoChar);
    glfwSetScrollCallback(demo->window, demoScroll);

    int w,h;
    glfwGetFramebufferSize(demo->window, &w, &h);
    demo->fbSize = (vec2s){{w,h}};
    
    return demo;
}

void demoDestroy()
{
    glfwDestroyWindow(demo->window);
    glfwTerminate();
    if(demo)
    {
        free((void*)demo);
        demo = NULL;
    }
}

static void demoUpdateButtons(size_t count, Button* buttons)
{
    for (size_t i = 0; i < count; i++)
    {
        buttons[i].pressed = buttons[i].down && !buttons[i].last;
        buttons[i].last = buttons[i].down;
    }
}

void demoRun()
{
    demo->cpuTime = 0;

    if(!demo)
        return;
    
    demo->init();

    glfwSetTime(0.0f);
    double lastFrame = glfwGetTime();
    demo->frameDelta = 0.0f;
    while (glfwWindowShouldClose(demo->window) == GL_FALSE)
    {
        double now = glfwGetTime();
        demo->frameDelta = now - lastFrame;
        lastFrame = now;
        demo->renderPass = -1;

        //input and update
        demoUpdateButtons(GLFW_KEY_LAST, demo->keys);
        demoUpdateButtons(GLFW_MOUSE_BUTTON_LAST, demo->mouse.buttons);
        demo->update();
        demo->mouse.delta = GLMS_VEC2_ZERO;

        
        //rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //3D
        demo->renderPass = PASS_3D;
        demo->render();

        //2D
        demo->renderPass = PASS_2D;
        demo->render(); // No draw calls here,set up buffers
        
        /* render ui from a demo here */
        demo->renderPass = PASS_UI;
        demo->render(); // PASS_UI

        demo->renderPass = PASS_FLUSH;
        demo->render(); // PASS_FLUSH

        demo->cpuTime = glfwGetTime() - now;
        

        //poll & swap
        glfwPollEvents();
        glfwSwapBuffers(demo->window);

        demo->mouse.wheelDelta = 0.0f;
    }
    
    demo->terminate();
}