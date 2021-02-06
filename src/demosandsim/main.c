#include "lgl/demo.h"
#include <time.h>

#define SLOT_SIZE 2
#define GRID_SIZE 512

const int slotsWidth = GRID_SIZE / SLOT_SIZE;
const int numOfSlots = (GRID_SIZE*GRID_SIZE) / SLOT_SIZE;
const float gravity = 100.0f;
const char* elementNames[] = {
    "Air",
    "Sand",
    "Stone",
    "Water"
};

static Demo* d = NULL;

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
    vec4 color;
}Element;

typedef void (*CreateElemFN)(Element* e);
typedef void (*UpdateElemFN)(int x, int y);

//vertex uniforms
static mat4 model;
static int modelLoc;
static mat4 proj;
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
    e->id = AIR;
    e->velocity = 0.0f;
    e->lifetime = 0.0f;
    glm_vec4((vec3){0.0f,0.0f,0.0f}, 0.0f, e->color);
}

void Sand(Element* e)
{
    e->id = SAND;
    e->lifetime = 0.0f;
    e->velocity = 2.0f;
    glm_vec4((vec3){0.9f,0.76f,0.16f}, 1.0f, e->color);
}

void Stone(Element* e)
{
    e->id = STONE;
    e->lifetime = 0.0f;
    e->velocity = 0.0f;
    glm_vec4((vec3){0.35f,0.35f,0.40f}, 1.0f, e->color);
}

void Water(Element* e)
{
    e->id = WATER;
    e->lifetime = 0.0f;
    e->velocity = 1.0f;
    glm_vec4((vec3){0.05f,0.2f,0.9f}, 1.0f, e->color);
}

void updateSand(int x, int y)
{
    if(y+1 == slotsWidth)
        return;
    
    // find out the cell we can go
    int xdest = x;
    int ydest = y;
    int newPosIndex = 0;

    //3places can go
    int down = Index(x, y+1);
    int downr = Index(x+1, y+1);
    int downl = Index(x-1, y+1);

    int r = Random(0, 10);

    if(grid.elementsGrid[down].id == AIR)
    {
        ydest++;
        newPosIndex = down;
    }else if(r > 5)
    {
        if(grid.elementsGrid[downr].id == AIR)
        {
            xdest++;
            ydest++;
            newPosIndex = downr;
        }else if(grid.elementsGrid[downl].id == AIR)
        {
            xdest--;
            ydest++;
            newPosIndex = downl;
        }else
        {
            return;
        }
    }else
    {
        if(grid.elementsGrid[downl].id == AIR)
        {
            xdest--;
            ydest++;
            newPosIndex = downr;
        }else if(grid.elementsGrid[downr].id == AIR)
        {
            xdest++;
            ydest++;
            newPosIndex = downl;
        }else
        {
            return;
        }
    }
    
    
    //move
    int thisIndex = Index(x, y);
    grid.elementsGrid[thisIndex].velocity -= gravity * d->frameDelta;
    if(grid.elementsGrid[thisIndex].velocity <= 0.0f)
    {
        createElemFuncs[SAND](&grid.elementsGrid[newPosIndex]);
        createElemFuncs[AIR](&grid.elementsGrid[thisIndex]);
        grid.elementsGrid[thisIndex].velocity = 2.0f;
    }
}

void updateStone(int x, int y)
{
}

void updateWater(int x, int y)
{
    if(y+1 == slotsWidth)
        return;
    
    // find out the cell we can go
    int xdest = x;
    int ydest = y;
    int newPosIndex = 0;

    //5places can go
    int down = Index(x, y+1);
    int downr = Index(x+1, y+1);
    int downl = Index(x-1, y+1);
    int right = Index(x+1, y);
    int left = Index(x-1,y);

    //int r = Random(0, 10);

    if(grid.elementsGrid[down].id == AIR)
    {
        ydest++;
        newPosIndex = down;
    }else if(grid.elementsGrid[downr].id == AIR)
    {
        xdest++;
        ydest++;
        newPosIndex = downr;
    }else if(grid.elementsGrid[downl].id == AIR)
    {
        xdest--;
        ydest++;
        newPosIndex = downl;
    }else if(grid.elementsGrid[right].id == AIR)
    {
        xdest++;
        newPosIndex = right;
    }else if(grid.elementsGrid[left].id == AIR)
    {
        xdest--;
        newPosIndex = left;
    }else
    {
        return;
    }
    
    //move
    int thisIndex = Index(x, y);
    grid.elementsGrid[thisIndex].velocity -= gravity * d->frameDelta;
    if(grid.elementsGrid[thisIndex].velocity <= 0.0f)
    {
        createElemFuncs[SAND](&grid.elementsGrid[newPosIndex]);
        createElemFuncs[AIR](&grid.elementsGrid[thisIndex]);
        grid.elementsGrid[thisIndex].velocity = 1.0f;
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

int addVertexColor(int index, vec3 v, vec4 color)
{
    vertexBuffer[index] = v[0];
    vertexBuffer[index+1] = v[1];
    vertexBuffer[index+2] = v[2];
    vertexBuffer[index+3] = color[0];
    vertexBuffer[index+4] = color[1];
    vertexBuffer[index+5] = color[2];
    vertexBuffer[index+6] = color[3];
    return index+7;
}

void drawElement(int x, int y, int w, int h, vec4 color)
{
    int index = vertCount;
    index = addVertexColor(index, (vec3){x, y, -1.0f}, color);
    index = addVertexColor(index, (vec3){x+w, y, -1.0f}, color);
    index = addVertexColor(index, (vec3){x, y+h, -1.0f}, color);
    index = addVertexColor(index, (vec3){x, y+h, -1.0f}, color);
    index = addVertexColor(index, (vec3){x+w, y, -1.0f}, color);
    index = addVertexColor(index, (vec3){x+w, y+h, -1.0f}, color);
    vertCount = index;
    worldVertCount += 6;
}

void initGrid()
{
    grid.xmin = (d->winSize[0] - GRID_SIZE) / 2;
    grid.xmax = grid.xmin + GRID_SIZE;
    grid.ymin = (d->winSize[1] - GRID_SIZE) / 2;
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
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj[0]);

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
    nvgFontSize(d->vg, 16.0f);
	nvgTextAlign(d->vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
	nvgFillColor(d->vg, nvgRGBA(255,255,255,200));
	nvgText(d->vg, x, y, txt, NULL);
}

static void renderUI()
{
    char debugInfo[64];
    sprintf(debugInfo, "Verts: %d, World verts: %d", vertCount, worldVertCount);
    drawText(0.0f, 0.0f, debugInfo);

    drawText(0.0f, 20.0f, elementNames[selectedElement]);
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
    
    glm_mat4_identity(model);
    glm_mat4_identity(proj);
    glm_ortho(0.0f, d->fbSize[0], d->fbSize[1], 0.0f, 0.1f, 10.0f, proj);    

    //glPointSize(2.0f);
    vertexBuffer = (float*)malloc(sizeof(float)*numOfSlots*7*6);
    memset(vertexBuffer, 0, sizeof(float)*numOfSlots*7*6);
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

    printf("Demo sand simulation terminated\n");
}

void update()
{
     if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].down)
    {
        int mx = (int)d->mouse.position[0];
        int my = (int)d->mouse.position[1];
        if(mx > grid.xmin && mx < grid.xmax && my > grid.ymin && my < grid.ymax)
        {
            int gridx = (mx - grid.xmin) / SLOT_SIZE;
            int gridy = (my - grid.ymin) / SLOT_SIZE;
            int index = Index(gridx, gridy);
            if(grid.elementsGrid[index].id == AIR)
            {
                createElemFuncs[selectedElement](&grid.elementsGrid[index]);
            }
        }
    }
    if(d->keys[GLFW_KEY_UP].pressed)
    {
        selectedElement++;
        if(selectedElement >= LAST_ELEM)
            selectedElement = SAND;
    }
    if(d->keys[GLFW_KEY_DOWN].pressed)
    {
        selectedElement--;
        if(selectedElement <= AIR)
            selectedElement = WATER;
    }
    
    updateGrid();
}

void render()
{
    switch (d->renderPass)
    {
    case PASS_3D:
        break;
    case PASS_2D:
        drawGrid();
        break;
    case PASS_UI:
        renderUI();
        break;
    case PASS_FLUSH:
        renderGrid();
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
