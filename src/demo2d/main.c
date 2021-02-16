#include "lgl/demo.h"
#include "lglui/demoui.h"

typedef void (*PanelAction)();

typedef struct Panel
{
    vec2s position;
    vec2s size;
    float alphas[3];
    vec3s colors[3];
    int color;
    int alpha;
    int alphaLoc;
    GLuint vbo;
    PanelAction clicked;
    double clickedDelay;
}Panel;

static Demo* d = NULL;
static Demoui* dui = NULL;
static Shader* simple = NULL;
static GLuint vao;
static int colorLoc;
static mat4s model;
static int modelLoc;
static mat4s proj;
static int projLoc;
static Panel panel;

static void panelCreate()
{
    panel.position = (vec2s){{10.0f, 10.0f}};
    panel.size = (vec2s){{ 100.0f, 50.0f }};
    panel.alpha = 0;
    panel.alphas[0] = 0.2f;
    panel.alphas[1] = 0.8f;
    panel.alphas[2] = 0.9f;
    panel.color = 0;
    panel.colors[0] = (vec3s){{1.0f, 0.0f, 0.0f}};
    panel.colors[1] = (vec3s){{0.0f, 1.0f, 0.0f}};
    panel.colors[2] = (vec3s){{0.0f, 0.0f, 1.0f}};
    panel.clickedDelay = 0.0f;
    
    float xmin = 0.0f;
    float xmax = panel.size.x;
    float ymin = 0.0f;
    float ymax = panel.size.y;

    const float vertbuffer[] = 
    {
        xmin, ymin, -1.0f,
        xmax, ymax, -1.0f,
        xmin, ymax, -1.0f,
        xmax, ymax, -1.0f,
        xmin, ymin, -1.0f,
        xmax, ymin, -1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &panel.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, panel.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, vertbuffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static bool panelHover()
{
    float xmax = panel.position.x + panel.size.x;
    float ymax = panel.position.y + panel.size.y;
    float mx = d->mouse.position.x;
    float my = d->mouse.position.y;
    return mx > panel.position.x && mx < xmax && my > panel.position.y && my < ymax;
}

static void OnPanelClicked()
{
    printf("Panel was clicked\n");
}

void init()
{
    printf("Demo2d initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    simple = shaderCreate("assets/shaders/simple2d.vert", "assets/shaders/simple2d.frag");
    shaderUse(simple);
    colorLoc = glGetUniformLocation(simple->program, "color");
    modelLoc = glGetUniformLocation(simple->program, "model");
    projLoc = glGetUniformLocation(simple->program, "proj");
    glUseProgram(0);

    panelCreate();
    panel.alphaLoc = glGetUniformLocation(simple->program, "alpha");
    panel.clicked = OnPanelClicked;
    
    model = glms_mat4_identity();
    model = glms_translate_make((vec3s){{panel.position.x, panel.position.y, -1.0f}});
    proj = glms_mat4_identity();
    proj = glms_ortho(0.0f, d->fbSize.x, d->fbSize.y, 0.0f, 0.1f, 100.0f);

    dui = demouiInit(d);
}

void terminate()
{
    demouiTerminate();
    glDeleteBuffers(1, &panel.vbo);
    glDeleteVertexArrays(1, &vao);
    shaderDestroy(simple);
    printf("Demo2d terminated\n");
}


void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->keys[GLFW_KEY_F3].pressed)
        demouiToggleShowGraphs();
    
    if(d->keys[GLFW_KEY_F4].pressed)
        demouiToggleGraph();
    
    if(panel.clickedDelay > 0.0f){
        panel.clickedDelay -= d->frameDelta;
        if(panel.clickedDelay < 0.0f)
            panel.clickedDelay = 0.0f;
    }

    if(panelHover() && panel.clickedDelay == 0.0f)
    {
        panel.alpha = 1;
        panel.color = 1;
        if(d->mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].pressed)
        {
            panel.alpha = 2;
            panel.color = 2;
            panel.clicked();
            panel.clickedDelay = 1.0f; //1 second 
        }
    }else if(panel.clickedDelay == 0.0f)
    {
        panel.alpha = 0;
        panel.color = 0; 
    }
}

void render()
{
    if(d->renderPass == PASS_3D)
    {
        demouiStartGPUTimer();
    }else if(d->renderPass == PASS_2D)
    {
        //glViewport(0, 0, (int)d->fbSize[0], (int)d->fbSize[1]);
        //glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shaderUse(simple);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.raw[0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.raw[0]);
        glUniform1f(panel.alphaLoc, panel.alphas[panel.alpha]);
        glUniform3fv(colorLoc, 1, panel.colors[panel.color].raw);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, panel.vbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisable(GL_BLEND);
        glUseProgram(0);
    }else if(d->renderPass == PASS_UI && dui->showGraphs)
    {
        demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);
    }else if(d->renderPass == PASS_FLUSH)
    {
        demouiEndRender(d->winSize.x - 200 - 5, 5);
        demouiUpdateGraphs(d->cpuTime, d->frameDelta);
    }
}

int main(void)
{
    d = demoCreate(init, terminate, update, render);
    demoRun();
    demoDestroy();

    return 0;
}