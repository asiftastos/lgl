#ifndef LGLUI_HEADER_H
#define LGLUI_HEADER_H

#include "nanovg.h"

typedef struct Demoui
{
    NVGcontext* vg;
    int vgFontNormal;
    bool showGraphs;
}Demoui;

void demouiInit();
void demouiTerminate();
void demouiToggleShowGraphs();
void demouiToggleGraph();
void demouiBeginRender(float winWidth, float winHeight, float pixelRatio);

//pass (demo->winSize[0] - 200 - 5, 5) for render in the top right corner if enabled
void demouiEndRender(float graphX, float graphY);

//call first of all on PASS_3D
void demouiStartGPUTimer();

// call last of all on PASS_FLUSH.cpuTime can be taken from the demo struct
void demouiUpdateGraphs(float cpuTime, float frameDelta);

#endif // LGLUI_HEADER_H