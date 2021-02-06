#include "demo.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#define GRAPH_HISTORY_COUNT 100
#define GPU_QUERY_COUNT 5

typedef enum GraphRenderStyle {
    GRAPH_RENDER_FPS,
    GRAPH_RENDER_MS,
    GRAPH_RENDER_PERCENT
}GraphRenderStyle;

typedef struct PerfGraph {
	int style;
	char name[32];
	float values[GRAPH_HISTORY_COUNT];
	int head;
}PerfGraph;

typedef struct GPUtimer
{
	int supported;
	int cur, ret;
	unsigned int queries[GPU_QUERY_COUNT];
}GPUtimer;

static Demo* demo = NULL;
static bool showGraphs = false;
static int graphIndex = 0;
static PerfGraph* perfGraphs;  //[fps, cpu, gpu]
static GPUtimer* gpuTimer;

static void demoErrorCallback(int error, const char* desc)
{
    _CRT_UNUSED(error);
    printf("GLFW error: %s\n", desc);
}

static void demoFramebufferSize(GLFWwindow* win, int w, int h)
{
    _CRT_UNUSED(win);
    glm_vec2((vec2){w, h}, demo->fbSize);
    glViewport(0, 0, w, h);
}

static void demoWindowSize(GLFWwindow* win, int w, int h)
{
    _CRT_UNUSED(win);
    glm_vec2((vec2){w, h}, demo->winSize);
}

static void demoCursorPos(GLFWwindow* win, double x, double y)
{
    _CRT_UNUSED(win);
    glm_vec2_sub((vec2){x, y}, demo->mouse.position, demo->mouse.delta);
    glm_vec2((vec2){x,y}, demo->mouse.position);
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

static float getGraphAverage(PerfGraph* fps)
{
	int i;
	float avg = 0;
	for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
		avg += fps->values[i];
	}
	return avg / (float)GRAPH_HISTORY_COUNT;
}

static void initGraphs()
{
    perfGraphs = (PerfGraph*)malloc(sizeof(PerfGraph) * 3);
    assert(perfGraphs != NULL);

	memset(&perfGraphs[0], 0, sizeof(PerfGraph));
	perfGraphs[0].style = GRAPH_RENDER_FPS;
	strncpy(perfGraphs[0].name, "Frame Time", sizeof(perfGraphs[0].name));
	perfGraphs[0].name[sizeof(perfGraphs[0].name)-1] = '\0';

    memset(&perfGraphs[1], 0, sizeof(PerfGraph));
	perfGraphs[1].style = GRAPH_RENDER_MS;
	strncpy(perfGraphs[1].name, "CPU Time", sizeof(perfGraphs[1].name));
	perfGraphs[1].name[sizeof(perfGraphs[1].name)-1] = '\0';

    memset(&perfGraphs[2], 0, sizeof(PerfGraph));
	perfGraphs[2].style = GRAPH_RENDER_MS;
	strncpy(perfGraphs[2].name, "GPU Time", sizeof(perfGraphs[2].name));
	perfGraphs[2].name[sizeof(perfGraphs[2].name)-1] = '\0';
}

static void updateGraph(PerfGraph* fps, float frameTime)
{
	fps->head = (fps->head+1) % GRAPH_HISTORY_COUNT;
	fps->values[fps->head] = frameTime;
}

static void drawGraph(float x, float y, PerfGraph* fps)
{
	int i;
	float avg, w, h;
	char str[64];

	avg = getGraphAverage(fps);

	// size of graphs
	w = 200;
	h = 35;

	nvgBeginPath(demo->vg);
	nvgRect(demo->vg, x,y, w,h);
	nvgFillColor(demo->vg, nvgRGBA(0,0,0,128));
	nvgFill(demo->vg);

	nvgBeginPath(demo->vg);
	nvgMoveTo(demo->vg, x, y+h);
	if (fps->style == GRAPH_RENDER_FPS) {
		for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
			float v = 1.0f / (0.00001f + fps->values[(fps->head+i) % GRAPH_HISTORY_COUNT]);
			float vx, vy;
			if (v > 80.0f) v = 80.0f;
			vx = x + ((float)i/(GRAPH_HISTORY_COUNT-1)) * w;
			vy = y + h - ((v / 80.0f) * h);
			nvgLineTo(demo->vg, vx, vy);
		}
	} else if (fps->style == GRAPH_RENDER_PERCENT) {
		for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
			float v = fps->values[(fps->head+i) % GRAPH_HISTORY_COUNT] * 1.0f;
			float vx, vy;
			if (v > 100.0f) v = 100.0f;
			vx = x + ((float)i/(GRAPH_HISTORY_COUNT-1)) * w;
			vy = y + h - ((v / 100.0f) * h);
			nvgLineTo(demo->vg, vx, vy);
		}
	} else {
		for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
			float v = fps->values[(fps->head+i) % GRAPH_HISTORY_COUNT] * 1000.0f;
			float vx, vy;
			if (v > 20.0f) v = 20.0f;
			vx = x + ((float)i/(GRAPH_HISTORY_COUNT-1)) * w;
			vy = y + h - ((v / 20.0f) * h);
			nvgLineTo(demo->vg, vx, vy);
		}
	}
	nvgLineTo(demo->vg, x+w, y+h);
	nvgFillColor(demo->vg, nvgRGBA(255,192,0,128));
	nvgFill(demo->vg);

	nvgFontFace(demo->vg, "sans");

	if (fps->name[0] != '\0') {
		nvgFontSize(demo->vg, 12.0f);
		nvgTextAlign(demo->vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
		nvgFillColor(demo->vg, nvgRGBA(240,240,240,192));
		nvgText(demo->vg, x+3,y+3, fps->name, NULL);
	}

	if (fps->style == GRAPH_RENDER_FPS) {
		nvgFontSize(demo->vg, 15.0f);
		nvgTextAlign(demo->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
		nvgFillColor(demo->vg, nvgRGBA(240,240,240,255));
		sprintf(str, "%.2f FPS", 1.0f / avg);
		nvgText(demo->vg, x+w-3,y+3, str, NULL);

		nvgFontSize(demo->vg, 13.0f);
		nvgTextAlign(demo->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_BASELINE);
		nvgFillColor(demo->vg, nvgRGBA(240,240,240,160));
		sprintf(str, "%.2f ms", avg * 1000.0f);
		nvgText(demo->vg, x+w-3,y+h-3, str, NULL);
	}
	else if (fps->style == GRAPH_RENDER_PERCENT) {
		nvgFontSize(demo->vg, 15.0f);
		nvgTextAlign(demo->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
		nvgFillColor(demo->vg, nvgRGBA(240,240,240,255));
		sprintf(str, "%.1f %%", avg * 1.0f);
		nvgText(demo->vg, x+w-3,y+3, str, NULL);
	} else {
		nvgFontSize(demo->vg, 15.0f);
		nvgTextAlign(demo->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
		nvgFillColor(demo->vg, nvgRGBA(240,240,240,255));
		sprintf(str, "%.2f ms", avg * 1000.0f);
		nvgText(demo->vg, x+w-3,y+3, str, NULL);
	}
}

static void initGPUTimer(GPUtimer* timer)
{
	memset(timer, 0, sizeof(*timer));

    timer->supported = glfwExtensionSupported("GL_ARB_timer_query");
	if (timer->supported) {
#ifndef GL_ARB_timer_query
		if (!glGetQueryObjectui64v) {
			timer->supported = GL_FALSE;
			return;
		}
#endif
		glGenQueries(GPU_QUERY_COUNT, timer->queries);
	}
}

static void startGPUTimer(GPUtimer* timer)
{
	if (!timer->supported)
		return;
	glBeginQuery(GL_TIME_ELAPSED, timer->queries[timer->cur % GPU_QUERY_COUNT] );
	timer->cur++;
}

static int stopGPUTimer(GPUtimer* timer, float* times, int maxTimes)
{
	NVG_NOTUSED(times);
	NVG_NOTUSED(maxTimes);

	GLint available = 1;
	int n = 0;
	if (!timer->supported)
		return 0;

	glEndQuery(GL_TIME_ELAPSED);
	while (available && timer->ret <= timer->cur) {
		// check for results if there are any
		glGetQueryObjectiv(timer->queries[timer->ret % GPU_QUERY_COUNT], GL_QUERY_RESULT_AVAILABLE, &available);
		if (available) {
			GLuint64 timeElapsed = 0;
			glGetQueryObjectui64v(timer->queries[timer->ret % GPU_QUERY_COUNT], GL_QUERY_RESULT, &timeElapsed);
			timer->ret++;
			if (n < maxTimes) {
				times[n] = (float)((double)timeElapsed * 1e-9);
				n++;
			}
		}
	}
	return n;
}

Demo* demoCreate(DemoFunc init, DemoFunc terminate, DemoFunc update, DemoFunc render)
{
    demo = (Demo*)calloc(1, sizeof(Demo));
    assert(demo);

    glm_vec2((vec2){800, 600}, demo->winSize);
    demo->init = init;
    demo->terminate = terminate;
    demo->update = update;
    demo->render = render;
    demo->renderPass = -1;

    initGraphs();

    //glfw init
    glfwSetErrorCallback(demoErrorCallback);
    int r = glfwInit();
    assert(r == GLFW_TRUE);
    printf("GLFW: %s\n", glfwGetVersionString());

    //window init
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    demo->window = glfwCreateWindow(demo->winSize[0], demo->winSize[1], "Demo", NULL, NULL);
    assert(demo->window);
    const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int xpos = (vmode->width - demo->winSize[0]) / 2;
    int ypos = (vmode->height - demo->winSize[1]) / 2;
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

    int w,h;
    glfwGetFramebufferSize(demo->window, &w, &h);
    glm_vec2((vec2){w,h}, demo->fbSize);
    
    demo->vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    assert(demo->vg != NULL);
    demo->vgFontNormal = nvgCreateFont(demo->vg, "sans", "assets/fonts/Roboto-Regular.ttf");
    
    return demo;
}

void demoDestroy()
{
    printf("Average Frame Time: %.2f ms||", getGraphAverage(&perfGraphs[0]) * 1000.0f);
	printf("CPU Time: %.2f ms||", getGraphAverage(&perfGraphs[1]) * 1000.0f);
	printf("GPU Time: %.2f ms\n", getGraphAverage(&perfGraphs[2]) * 1000.0f);

    if(perfGraphs)
        free(perfGraphs);
    
    if(gpuTimer)
        free(gpuTimer);

    nvgDeleteGL3(demo->vg);
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
    double cpuTime = 0;

    if(!demo)
        return;
    
    demo->init();

    //GPU Timer
    gpuTimer = (GPUtimer*)malloc(sizeof(GPUtimer));
    initGPUTimer(gpuTimer);

    glfwSetTime(0.0f);
    double lastFrame = glfwGetTime();
    demo->frameDelta = 0.0f;
    while (glfwWindowShouldClose(demo->window) == GL_FALSE)
    {
        float gpuTimes[3];
        double now = glfwGetTime();
        demo->frameDelta = now - lastFrame;
        lastFrame = now;
        demo->renderPass = -1;

        //input and update
        demoUpdateButtons(GLFW_KEY_LAST, demo->keys);
        demoUpdateButtons(GLFW_MOUSE_BUTTON_LAST, demo->mouse.buttons);
        demo->update();
        glm_vec2((vec2){0.0f, 0.0f}, demo->mouse.delta);

        //show/hide graphs
        if(demo->keys[GLFW_KEY_F3].pressed)
            showGraphs = !showGraphs;
        //change between graphs [FPS, CPU, GPU]
        if(demo->keys[GLFW_KEY_F4].pressed)
            graphIndex = (graphIndex + 1) % 3;

        //rendering
        startGPUTimer(gpuTimer); //start the gpu timer measure drawing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //3D
        demo->renderPass = PASS_3D;
        demo->render();

        //2D
        demo->renderPass = PASS_2D;
        demo->render(); // No draw calls here,set up buffers

        //UI
        float pixelRatio = demo->fbSize[0] / demo->winSize[0];
        nvgBeginFrame(demo->vg, demo->winSize[0], demo->winSize[1], pixelRatio);
        
        /* render ui from a demo here */
        demo->renderPass = PASS_UI;
        demo->render(); // PASS_UI

        //render graphs
        if(showGraphs)
            drawGraph(demo->winSize[0] - 200 - 5, 5, &perfGraphs[graphIndex]);
        
        nvgEndFrame(demo->vg);

        demo->renderPass = PASS_FLUSH;
        demo->render(); // PASS_FLUSH

        //update graphs, we draw one at a time but update all.
        cpuTime = glfwGetTime() - now;
        updateGraph(&perfGraphs[0], demo->frameDelta);
        updateGraph(&perfGraphs[1], cpuTime);
        int n = stopGPUTimer(gpuTimer, gpuTimes, 3);
        for (int i = 0; i < n; i++)
			updateGraph(&perfGraphs[2], gpuTimes[i]);

        //poll & swap
        glfwPollEvents();
        glfwSwapBuffers(demo->window);
    }
    
    demo->terminate();
}