//#include <stdlib.h>
//#include <stdbool.h>
//#include <assert.h>
//#include "glad.h"
//#include "GLFW/glfw3.h"
#include "demo.h"
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "demoui.h"

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
static Demoui* demoui = NULL;
static int graphIndex;
static PerfGraph* perfGraphs;  //[fps, cpu, gpu]
static GPUtimer* gpuTimer;
static float gpuTimes[3];

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

	nvgBeginPath(demoui->vg);
	nvgRect(demoui->vg, x,y, w,h);
	nvgFillColor(demoui->vg, nvgRGBA(0,0,0,128));
	nvgFill(demoui->vg);

	nvgBeginPath(demoui->vg);
	nvgMoveTo(demoui->vg, x, y+h);
	if (fps->style == GRAPH_RENDER_FPS) {
		for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
			float v = 1.0f / (0.00001f + fps->values[(fps->head+i) % GRAPH_HISTORY_COUNT]);
			float vx, vy;
			if (v > 80.0f) v = 80.0f;
			vx = x + ((float)i/(GRAPH_HISTORY_COUNT-1)) * w;
			vy = y + h - ((v / 80.0f) * h);
			nvgLineTo(demoui->vg, vx, vy);
		}
	} else if (fps->style == GRAPH_RENDER_PERCENT) {
		for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
			float v = fps->values[(fps->head+i) % GRAPH_HISTORY_COUNT] * 1.0f;
			float vx, vy;
			if (v > 100.0f) v = 100.0f;
			vx = x + ((float)i/(GRAPH_HISTORY_COUNT-1)) * w;
			vy = y + h - ((v / 100.0f) * h);
			nvgLineTo(demoui->vg, vx, vy);
		}
	} else {
		for (i = 0; i < GRAPH_HISTORY_COUNT; i++) {
			float v = fps->values[(fps->head+i) % GRAPH_HISTORY_COUNT] * 1000.0f;
			float vx, vy;
			if (v > 20.0f) v = 20.0f;
			vx = x + ((float)i/(GRAPH_HISTORY_COUNT-1)) * w;
			vy = y + h - ((v / 20.0f) * h);
			nvgLineTo(demoui->vg, vx, vy);
		}
	}
	nvgLineTo(demoui->vg, x+w, y+h);
	nvgFillColor(demoui->vg, nvgRGBA(255,192,0,128));
	nvgFill(demoui->vg);

	nvgFontFace(demoui->vg, "sans");

	if (fps->name[0] != '\0') {
		nvgFontSize(demoui->vg, 12.0f);
		nvgTextAlign(demoui->vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
		nvgFillColor(demoui->vg, nvgRGBA(240,240,240,192));
		nvgText(demoui->vg, x+3,y+3, fps->name, NULL);
	}

	if (fps->style == GRAPH_RENDER_FPS) {
		nvgFontSize(demoui->vg, 15.0f);
		nvgTextAlign(demoui->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
		nvgFillColor(demoui->vg, nvgRGBA(240,240,240,255));
		sprintf(str, "%.2f FPS", 1.0f / avg);
		nvgText(demoui->vg, x+w-3,y+3, str, NULL);

		nvgFontSize(demoui->vg, 13.0f);
		nvgTextAlign(demoui->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_BASELINE);
		nvgFillColor(demoui->vg, nvgRGBA(240,240,240,160));
		sprintf(str, "%.2f ms", avg * 1000.0f);
		nvgText(demoui->vg, x+w-3,y+h-3, str, NULL);
	}
	else if (fps->style == GRAPH_RENDER_PERCENT) {
		nvgFontSize(demoui->vg, 15.0f);
		nvgTextAlign(demoui->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
		nvgFillColor(demoui->vg, nvgRGBA(240,240,240,255));
		sprintf(str, "%.1f %%", avg * 1.0f);
		nvgText(demoui->vg, x+w-3,y+3, str, NULL);
	} else {
		nvgFontSize(demoui->vg, 15.0f);
		nvgTextAlign(demoui->vg,NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
		nvgFillColor(demoui->vg, nvgRGBA(240,240,240,255));
		sprintf(str, "%.2f ms", avg * 1000.0f);
		nvgText(demoui->vg, x+w-3,y+3, str, NULL);
	}
}

static void startGPUTimer(GPUtimer* timer)
{
	if (!timer->supported)
		return;
	glBeginQuery(GL_TIME_ELAPSED, timer->queries[timer->cur % GPU_QUERY_COUNT] );
	timer->cur++;
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

static int stopGPUTimer(GPUtimer* timer, float* times, int maxTimes)
{
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


Demoui* demouiInit(Demo* dm)
{
	demo = dm;
	
    demoui = (Demoui*)calloc(1, sizeof(Demoui));
    assert(demoui);

    initGraphs();

    demoui->vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    assert(demoui->vg != NULL);
    demoui->vgFontNormal = nvgCreateFont(demoui->vg, "sans", "assets/fonts/Roboto-Regular.ttf");
    
    //GPU Timer
    gpuTimer = (GPUtimer*)malloc(sizeof(GPUtimer));
    initGPUTimer(gpuTimer);

	return demoui;
}

void demouiTerminate()
{
    printf("Average Frame Time: %.2f ms||", getGraphAverage(&perfGraphs[0]) * 1000.0f);
	printf("CPU Time: %.2f ms||", getGraphAverage(&perfGraphs[1]) * 1000.0f);
	printf("GPU Time: %.2f ms\n", getGraphAverage(&perfGraphs[2]) * 1000.0f);

    if(perfGraphs)
        free(perfGraphs);
    
    if(gpuTimer)
        free(gpuTimer);
    
    nvgDeleteGL3(demoui->vg);
    
    if(demoui)
        free((void*)demoui);
}

void demouiToggleShowGraphs()
{
    demoui->showGraphs = !demoui->showGraphs;
}

void demouiToggleGraph()
{
    graphIndex = (graphIndex + 1) % 3;
}

void demouiBeginRender(float winWidth, float winHeight, float pixelRatio)
{
    nvgBeginFrame(demoui->vg, winWidth, winHeight, pixelRatio);
}

void demouiEndRender(float graphX, float graphY)
{
    if(demoui->showGraphs)
        drawGraph(graphX, graphY, &perfGraphs[graphIndex]);

    nvgEndFrame(demoui->vg);
}

void demouiStartGPUTimer()
{
    startGPUTimer(gpuTimer); //start the gpu timer measure drawing
}

void demouiUpdateGraphs(float cpuTime, float frameDelta)
{
    //update graphs, we draw one at a time but update all.
    updateGraph(&perfGraphs[0], frameDelta);
    updateGraph(&perfGraphs[1], cpuTime);
    int n = stopGPUTimer(gpuTimer, gpuTimes, 3);
    for (int i = 0; i < n; i++)
	    updateGraph(&perfGraphs[2], gpuTimes[i]);
}