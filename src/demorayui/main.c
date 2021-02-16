#include "lgl/demo.h"
#include "lglui/demoui.h"
#define RAYGUI_STANDALONE
#include "lglui/raygui.h"

static Demo* d = NULL;
static Demoui* dui = NULL;
static bool valueEditMode = false;
static int value = 5;

//============================================================================================
//============================================================================================
//============================================================================================
void init()
{
    printf("Demorayui initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    dui = demouiInit(d);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
}

void terminate()
{
    demouiTerminate();
    printf("Demorayui terminated\n");
}

void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->keys[GLFW_KEY_F3].pressed)
        demouiToggleShowGraphs();
    
    if(d->keys[GLFW_KEY_F4].pressed)
        demouiToggleGraph();
}

void render()
{
    switch (d->renderPass)
    {
    case PASS_3D:
        demouiStartGPUTimer();
        break;
    case PASS_2D:
        break;
    case PASS_UI:
        {
            demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);

            Rectangle panelBounds = {1.0f, 1.0f, 100.0f, 50.0f};
            Rectangle dropDown = {10.0f, 100.0f, 100.0f, 50.0f};
            //GuiPanel(panelBounds);
            //GuiDrawText("Kostas", (Rectangle){panelBounds.x, panelBounds.y,panelBounds.width,panelBounds.height}, GUI_TEXT_ALIGN_CENTER, (Color){0, 0, 0, 255});
            //GuiLabel(panelBounds, "Kostas");
            //GuiGroupBox(panelBounds, "Debug");
            //bool buttoncloseclicked = GuiWindowBox(panelBounds, "Debug");
            if(GuiButton(panelBounds, "Exit"))
                glfwSetWindowShouldClose(d->window, GLFW_TRUE);
            
            //int active = 0;
            //GuiDropdownBox(dropDown, "DropME", &active, false);
            if(GuiValueBox(dropDown, NULL, &value, 0, 10, valueEditMode)) valueEditMode = !valueEditMode;
        }
        break;
    case PASS_FLUSH:
        {
            demouiEndRender(d->winSize.x - 200 - 5, 5);
            demouiUpdateGraphs(d->cpuTime, d->frameDelta);
        }
        break;
    default:
        break;
    }
}

int main(void)
{
    d = demoCreate(init, terminate, update, render);
    demoRun();
    demoDestroy();

    return 0;
}