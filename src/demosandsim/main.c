#include "lgl/demo.h"
#include "lglui/demoui.h"
#include <time.h>

#define SLOT_SIZE 4
#define GRID_SIZE 512

const int slotsWidth = GRID_SIZE / SLOT_SIZE;
const int numOfSlots = (GRID_SIZE*GRID_SIZE) / SLOT_SIZE;
const float gravity = 1.0f;

const char* elementNames[] = {
    "Air",
    "Sand",
    "Stone",
    "Water"
};

static Demo* d = NULL;
static Demoui* dui = NULL;

typedef enum ElementType
{
    AIR,
    SAND,
    STONE,
    WATER,
    LAST_ELEM
}ElementType;

// elements
typedef struct Element
{
    unsigned char id;
    float lifetime;
    float velocity;
    vec4s color;
    int newPos;
}Element;

typedef void (*CreateElemFN)(Element* e);
typedef void (*UpdateElemFN)(int x, int y);

//vertex uniforms
static mat4s model;
static int modelLoc;
static mat4s proj;
static int projLoc;

//grid
typedef struct Grid
{
    //screen coords
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    Element* elementsGrid;
}Grid;

Grid grid;
GLuint gridVao;
GLuint gridVbo;
GLuint worldVao;
GLuint worldVbo;
float* vertexBuffer = NULL;
int vertCount = 0;
int worldVertCount = 0;
Shader* sh;
CreateElemFN createElemFuncs[LAST_ELEM];
UpdateElemFN updateElemFuncs[LAST_ELEM];
int selectedElement = SAND;
int cursorRadius = 1;

int Random(int min, int max)
{
    time_t t;
    srand((unsigned int)time(&t));
    return (rand()%(abs(max - min) + 1) + min);
}

int Index(int x, int y)
{
    return y*slotsWidth+x;
}

void Air(Element* e)
{
    e->newPos = -1;
    e->id = AIR;
    e->velocity = 0.0f;
    e->lifetime = 0.0f;
    e->color = (vec4s){{0.0f,0.0f,0.0f,1.0f}};
}

void Sand(Element* e)
{
    e->newPos = -1;
    e->id = SAND;
    e->lifetime = 0.0f;
    e->velocity = 1.0f;
    e->color = (vec4s){{0.9f,0.76f,0.16f, 1.0f}};
}

void Stone(Element* e)
{
    e->newPos = -1;
    e->id = STONE;
    e->lifetime = 0.0f;
    e->velocity = 0.0f;
    e->color= (vec4s){{0.35f,0.35f,0.40f, 1.0f}};
}

void Water(Element* e)
{
    e->newPos = -1;
    e->id = WATER;
    e->lifetime = 0.0f;
    e->velocity = 1.5f;
    e->color = (vec4s){{0.05f,0.2f,0.9f, 1.0f}};
}

void updateSand(int x, int y)
{
    if(y+1 == slotsWidth)
        return;
    
    int thisIndex = Index(x, y);
    Element* thisElem = &grid.elementsGrid[thisIndex];
    float v = thisElem->velocity + gravity * d->frameDelta;
    
    if(thisElem->newPos == -1)
    {
        int down = Index(x, y + v);
        int vx = Random(0, 1) == 0 ? -1 : 1; //randomly select left/right

        if(grid.elementsGrid[down].id == AIR)
        {
            thisElem->newPos = down;
        }else if(grid.elementsGrid[Index( x + vx, y + v)].id == AIR)
        {
            thisElem->newPos = Index(x + vx, y + v);
        }else if(grid.elementsGrid[Index( x - vx, y + v)].id == AIR)
        {
            thisElem->newPos = Index(x - vx, y + v);
        }else
        {
            return; //it can't go anywhere
        }
    }

    if(grid.elementsGrid[thisElem->newPos].id == AIR)
    {
        thisElem->velocity = 1.0f;
        createElemFuncs[SAND](&grid.elementsGrid[thisElem->newPos]);
        createElemFuncs[AIR](&grid.elementsGrid[thisIndex]);
        //thisElem->newPos = -1;
    }
}

void updateStone(int x, int y)
{
}

void updateWater(int x, int y)
{
    if(y+1 == slotsWidth)
        return;
    
    int thisIndex = Index(x, y);
    Element* thisElem = &grid.elementsGrid[thisIndex];
    float v = thisElem->velocity + gravity * d->frameDelta;

    if(thisElem->newPos == -1)
    {
        //5places can go
        int down = Index(x, y+1);
        int vx = Random(0,1) == 0 ? -1 : 1;

        if(grid.elementsGrid[down].id == AIR)
        {
            thisElem->newPos = down;
        }else if(grid.elementsGrid[Index(x + vx, y + v)].id == AIR)
        {
            thisElem->newPos = Index(x + vx, y + v);
        }else if(grid.elementsGrid[Index(x - vx, y + v)].id == AIR)
        {
            thisElem->newPos = Index(x - vx, y + v);
        }else if(grid.elementsGrid[Index(x + vx, y)].id == AIR)
        {
            thisElem->newPos = Index(x + vx, y);
        }else if(grid.elementsGrid[Index(x - vx, y)].id == AIR)
        {
            thisElem->newPos = Index(x - vx, y);
        }else
        {
            return;
        }
    }
    
    
    if(grid.elementsGrid[thisElem->newPos].id == AIR)
    {
        thisElem->velocity = 1.5f;
        createElemFuncs[WATER](&grid.elementsGrid[thisElem->newPos]);
        createElemFuncs[AIR](&grid.elementsGrid[thisIndex]);
        //thisElem->newPos = -1;
    }
}

void initElements()
{
    createElemFuncs[AIR] = Air;
    createElemFuncs[SAND] = Sand;
    createElemFuncs[STONE] = Stone;
    createElemFuncs[WATER] = Water;

    updateElemFuncs[AIR] = NULL;
    updateElemFuncs[SAND] = updateSand;
    updateElemFuncs[STONE] = updateStone;
    updateElemFuncs[WATER] = updateWater;
}

int addVertexColor(int index, vec3s v, vec4s color)
{
    vertexBuffer[index] = v.x;
    vertexBuffer[index+1] = v.y;
    vertexBuffer[index+2] = v.z;
    vertexBuffer[index+3] = color.x;
    vertexBuffer[index+4] = color.y;
    vertexBuffer[index+5] = color.z;
    vertexBuffer[index+6] = color.w;
    return index+7;
}

void drawElement(int x, int y, int w, int h, vec4s color)
{
    int index = vertCount;
    index = addVertexColor(index, (vec3s){{x, y, -1.0f}}, color);
    index = addVertexColor(index, (vec3s){{x+w, y, -1.0f}}, color);
    index = addVertexColor(index, (vec3s){{x, y+h, -1.0f}}, color);
    index = addVertexColor(index, (vec3s){{x, y+h, -1.0f}}, color);
    index = addVertexColor(index, (vec3s){{x+w, y, -1.0f}}, color);
    index = addVertexColor(index, (vec3s){{x+w, y+h, -1.0f}}, color);
    vertCount = index;
    worldVertCount += 6;
}

void initGrid()
{
    grid.xmin = (d->winSize.x - GRID_SIZE) / 2;
    grid.xmax = grid.xmin + GRID_SIZE;
    grid.ymin = (d->winSize.y - GRID_SIZE) / 2;
    grid.ymax = grid.ymin + GRID_SIZE;

    float verts[] = {
        grid.xmin, grid.ymin, -1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        grid.xmax, grid.ymin, -1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        grid.xmax, grid.ymax, -1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        grid.xmin, grid.ymax, -1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &gridVao);
    glBindVertexArray(gridVao);
    glGenBuffers(1, &gridVbo);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 28, verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &worldVao);
    glBindVertexArray(worldVao);
    glGenBuffers(1, &worldVbo);
    glBindBuffer(GL_ARRAY_BUFFER, worldVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * numOfSlots * 6, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    worldVertCount = 0;

    grid.elementsGrid = (Element*)malloc(sizeof(Element)*numOfSlots);
    for (int y = slotsWidth-1; y > 0; --y)
    {
        for (int x = slotsWidth-1; x > 0; --x)
        {
            int index = Index(x, y);
            createElemFuncs[AIR](&grid.elementsGrid[index]);
        }
    }
}

static void updateGrid()
{
    //update from bottom to up
    for (int y = slotsWidth-1; y > 0; --y)
    {
        for (int x = slotsWidth-1; x > 0; --x)
        {
            int index = Index(x, y);
            int type = grid.elementsGrid[index].id;
            if(type == AIR)
                continue;
            
            if(updateElemFuncs[type] == NULL)
                continue;
            
            updateElemFuncs[type](x, y);
        }
    }
}

static void drawGrid()
{
    vertCount = 0;
    worldVertCount = 0;
    for (int y = slotsWidth-1; y > 0; --y)
    {
        for (int x = slotsWidth-1; x > 0; --x)
        {
            int i = Index(x,y);
            if(grid.elementsGrid[i].id == AIR)
                continue;
            int posX = grid.xmin + (x * SLOT_SIZE);
            int posY = grid.ymin + (y * SLOT_SIZE);
            drawElement(posX, posY, SLOT_SIZE, SLOT_SIZE, grid.elementsGrid[i].color);
        }
    }

    glBindVertexArray(worldVao);
    glBindBuffer(GL_ARRAY_BUFFER, worldVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertCount, vertexBuffer);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void renderGrid()
{
    //glViewport(0, 0, (int)d->fbSize[0], (int)d->fbSize[1]);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shaderUse(sh);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.raw[0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.raw[0]);

    glBindVertexArray(gridVao);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    //glBindVertexArray(0);

    glBindVertexArray(worldVao);
    glDrawArrays(GL_TRIANGLES, 0, worldVertCount);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void drawText(float x, float y, const char* txt)
{
    nvgFontSize(dui->vg, 16.0f);
	nvgTextAlign(dui->vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
	nvgFillColor(dui->vg, nvgRGBA(255,255,255,200));
	nvgText(dui->vg, x, y, txt, NULL);
}

static void renderUI()
{
    demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);

    char debugInfo[64];
    sprintf(debugInfo, "Verts: %d, World verts: %d", vertCount, worldVertCount);
    drawText(0.0f, 0.0f, debugInfo);

    char elemInfo[64];
    sprintf(elemInfo, "Element: %s, Cursor radius: %d", elementNames[selectedElement], cursorRadius);
    drawText(0.0f, 20.0f, elemInfo);
}

void init()
{
    printf("Demo sand simulation initialized\n");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    sh = shaderCreate("assets/shaders/sandsim.vert", "assets/shaders/sandsim.frag");
    shaderUse(sh);
    modelLoc = glGetUniformLocation(sh->program, "model");
    projLoc = glGetUniformLocation(sh->program, "proj");

    initElements();
    initGrid();
    
    model = glms_mat4_identity();
    proj = glms_ortho(0.0f, d->fbSize.x, d->fbSize.y, 0.0f, 0.1f, 10.0f);

    //glPointSize(2.0f);
    vertexBuffer = (float*)malloc(sizeof(float)*numOfSlots*7*6);
    memset(vertexBuffer, 0, sizeof(float)*numOfSlots*7*6);

    dui = demouiInit(d);
}

void terminate()
{
    if(vertexBuffer)
        free((void*)vertexBuffer);
    if(grid.elementsGrid)
        free((void*)grid.elementsGrid);
    
    glDeleteBuffers(1, &gridVbo);
    glDeleteVertexArrays(1, &gridVao);
    glDeleteBuffers(1, &worldVbo);
    glDeleteVertexArrays(1, &worldVao);
    shaderDestroy(sh);

    demouiTerminate();

    printf("Demo sand simulation terminated\n");
}

void update()
{
     if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->keys[GLFW_KEY_F3].pressed)
        demouiToggleShowGraphs();
    
    if(d->keys[GLFW_KEY_F4].pressed)
        demouiToggleGraph();
    
    if(d->keys[GLFW_KEY_UP].pressed)
    {
        selectedElement++;
        if(selectedElement == LAST_ELEM)
            selectedElement = AIR;
    }
    if(d->keys[GLFW_KEY_DOWN].pressed)
    {
        selectedElement--;
        if(selectedElement <= AIR)
            selectedElement = LAST_ELEM - 1;
    }
    if(d->keys[GLFW_KEY_RIGHT].pressed)
    {
        cursorRadius++;
        if(cursorRadius > 4)
            cursorRadius = 4;
    }
    if(d->keys[GLFW_KEY_LEFT].pressed)
    {
        cursorRadius--;
        if(cursorRadius < 0)
            cursorRadius = 0;
    }

    if(d->mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].down)
    {
        int mx = (int)d->mouse.position.x;
        int my = (int)d->mouse.position.y;
        if(mx > grid.xmin && mx < grid.xmax && my > grid.ymin && my < grid.ymax)
        {
            int gridx = (mx - grid.xmin) / SLOT_SIZE;
            int gridy = (my - grid.ymin) / SLOT_SIZE;
            for(int x = gridx - cursorRadius; x <= gridx + cursorRadius; x++)
            {
                for(int y = gridy - cursorRadius; y <= gridy + cursorRadius; y++)
                {
                    createElemFuncs[selectedElement](&grid.elementsGrid[Index(x,y)]);
                }
            }
            //createElemFuncs[selectedElement](&grid.elementsGrid[Index(gridx,gridy)]);
        }
    }
    
    updateGrid();
}

void render()
{
    switch (d->renderPass)
    {
    case PASS_3D:
        demouiStartGPUTimer();
        break;
    case PASS_2D:
        drawGrid();
        break;
    case PASS_UI:
        {
            renderUI();
        }
        break;
    case PASS_FLUSH:
        {
            renderGrid();

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
