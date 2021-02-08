#include "lgl/demo.h"
#include "lglui/demoui.h"

#define CLITERAL(type)      (type)

//colors
#define BLANK    CLITERAL(Color){ 0, 0, 0, 0 }

//keys
#define KEY_LEFT_SHIFT GLFW_KEY_LEFT_SHIFT 
#define KEY_RIGHT_SHIFT GLFW_KEY_RIGHT_SHIFT 

static const char *CodepointToUtf8(int codepoint, int *byteLength);

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_STANDALONE
#include "demorayui/raygui.h"

static Demo* d = NULL;
static Demoui* dui = NULL;

static Vector2 GetMousePosition(void)
{
    Vector2 position = { 0 };
    
    position.x = d->mouse.position[0];
    position.y = d->mouse.position[1];
    
    return position;
}

static int GetMouseWheelMove(void)
{
    // TODO: Mouse wheel movement variation, reseted every frame
    
    return 0;
}

static bool IsMouseButtonDown(int button)
{
    if(d->mouse.buttons[button].down)
        return true;
    
    return false;
}

static bool IsMouseButtonPressed(int button)
{
    if(d->mouse.buttons[button].pressed)
        return true;
    
    return false;
}

static bool IsMouseButtonReleased(int button)
{
    if(d->mouse.buttons[button].pressed)
        return true;
    
    return false;
}

static bool IsKeyDown(int key)
{
    if(d->keys[key].down)
        return true;
    
    return false;
}

static bool IsKeyPressed(int key)
{
    if(d->keys[key].pressed)
        return true;
    
    return false;
}

// USED IN: GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()
static int GetKeyPressed(void)
{
    // TODO: Return last key pressed (up->down) in the frame
    
    return 0;
}

static int GetCharPressed(void)
{
    return 0;
}

static void DrawRectangle(int x, int y, int width, int height, Color color)
{ 
    nvgBeginPath(dui->vg);
	nvgRect(dui->vg, x,y, width, height);
	nvgFillColor(dui->vg, nvgRGBA(color.r,color.g,color.b,color.a));
	nvgFill(dui->vg);
}

// USED IN: GuiColorPicker()
static void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4)
{
    // TODO: Draw rectangle with gradients (4 vertex colors) on the screen
}

// USED IN: GuiDropdownBox(), GuiScrollBar()
static void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{ 
    // TODO: Draw triangle on the screen, required for arrows
}

// USED IN: GuiImageButtonEx()
static void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint)
{
    // TODO: Draw texture (piece defined by source rectangle) on screen
}

// USED IN: GuiTextBoxMulti()
static void DrawTextRec(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
    // TODO: Draw text limited by a rectangle. This advance function wraps the text inside the rectangle
}

// USED IN: GuiLoadStyleDefault()
static Font GetFontDefault(void)
{
    Font font = { 0 };
    
    // TODO: Return default rendering Font for the UI
    
    return font; 
}

// USED IN: GetTextWidth(), GuiTextBoxMulti()
static Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing) 
{ 
    Vector2 size = { 0 };
    
    nvgTextLetterSpacing(dui->vg, spacing);
    nvgFontSize(dui->vg, fontSize);
    float bounds[4];
    nvgTextBounds(dui->vg, 0.0f, 0.0f, text, NULL, bounds);
    size.x = bounds[2] - bounds[0];
    size.y = bounds[3] - bounds[1];
    
    return size;
}

// USED IN: GuiDrawText()
static void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
{
    nvgFontFace(dui->vg, "sans");
    nvgFontSize(dui->vg, fontSize);
	nvgTextAlign(dui->vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    nvgTextLetterSpacing(dui->vg, spacing);
	nvgFillColor(dui->vg, nvgRGBA(tint.r,tint.g,tint.b,tint.a));
	nvgText(dui->vg, position.x,position.y, text, NULL);
}

static Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount)
{
    Font font = { 0 };
    
    // TODO: Load a new font from a file
    
    return font; 
}

static char *LoadText(const char *fileName)
{
    // TODO: Load text file data, used by GuiLoadStyle() to load characters list required on Font generation,
    // this is a .rgs feature, probably this function is not required in most cases

    return NULL;
}

static const char *GetDirectoryPath(const char *filePath)
{
    // TODO: Get directory path for .rgs file, required to look for a possible .ttf/.otf font file referenced,
    // this is a .rgs feature, probably this function is not required in most cases
    
    return NULL;
}


//============================================================================================
//============================================================================================
//============================================================================================
void init()
{
    printf("Demorayui initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    dui = demouiInit();

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
            demouiBeginRender(d->winSize[0], d->winSize[1], d->winSize[0] / d->fbSize[0]);

            Rectangle panelBounds = {10.0f, 10.0f, 200.0f, 100.0f};
            //GuiPanel(panelBounds);
            //GuiDrawText("Kostas", (Rectangle){panelBounds.x, panelBounds.y,panelBounds.width,panelBounds.height}, GUI_TEXT_ALIGN_CENTER, (Color){0, 0, 0, 255});
            //GuiLabel(panelBounds, "Kostas");
            //GuiGroupBox(panelBounds, "Debug");
            //bool buttoncloseclicked = GuiWindowBox(panelBounds, "Debug");
            if(GuiButton(panelBounds, "Exit"))
                glfwSetWindowShouldClose(d->window, GLFW_TRUE);
        }
        break;
    case PASS_FLUSH:
        {
            demouiEndRender(d->winSize[0] - 200 - 5, 5);
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