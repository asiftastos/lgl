#include "lgl/demo.h"

typedef void (*PanelAction)();

typedef struct Panel
{
    vec2 position;
    vec2 size;
    float alphas[3];
    vec3 colors[3];
    int color;
    int alpha;
    int alphaLoc;
    GLuint vbo;
    mat3 model;
    PanelAction clicked;
    double clickedDelay;
}Panel;

static Demo* d = NULL;
static Shader* simple = NULL;
static GLuint vao;
static int colorLoc;
static mat4 model;
static int modelLoc;
static mat4 proj;
static int projLoc;
static Panel panel;

static void panelCreate()
{
    glm_vec2((vec2){10.0f, 10.0f}, panel.position);
    mat3 invmodel;
    glm_mat3_identity(invmodel);
    glm_translate2d(invmodel, panel.position);
    glm_mat3_inv(invmodel, panel.model);
    glm_vec2((vec2){ 100.0f, 50.0f}, panel.size);
    panel.alpha = 0;
    panel.alphas[0] = 0.2f;
    panel.alphas[1] = 0.8f;
    panel.alphas[2] = 0.9f;
    panel.color = 0;
    glm_vec3((vec4){1.0f, 0.0f, 0.0f, 1.0f}, panel.colors[0]);
    glm_vec3((vec4){0.0f, 1.0f, 0.0f, 1.0f}, panel.colors[1]);
    glm_vec3((vec4){0.0f, 0.0f, 1.0f, 1.0f}, panel.colors[2]);
    panel.clickedDelay = 0.0f;
    
    float xmin = 0.0f;
    float xmax = panel.size[0];
    float ymin = 0.0f;
    float ymax = panel.size[1];

    const float vertbuffer[] = 
    {
        xmin, ymin, -1.0f,
        xmax, ymax, -1.0f,
        xmin, ymax, -1.0f,
        xmax, ymax, -1.0f,
        xmin, ymin, -1.0f,
        xmax, ymin, -1.0f
    };

    glGenBuffers(1, &panel.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, panel.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, vertbuffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);
}

static bool panelHover()
{
    float xmax = panel.position[0] + panel.size[0];
    float ymax = panel.position[1] + panel.size[1];
    float mx = d->mouse.position[0];
    float my = d->mouse.position[1];
    return mx > panel.position[0] && mx < xmax && my > panel.position[1] && my < ymax;
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
    panelCreate();
    panel.alphaLoc = glGetUniformLocation(simple->program, "alpha");
    panel.clicked = OnPanelClicked;
    
    //printf("Alpha location: %d\n", alphaLoc);
    glm_mat4_identity(model);
    //glm_scale(model, (vec3){2.5f, 1.5f, 1.0f});
    //glm_translate(model, (vec3){panel.position[0], panel.position[1], 0.0f});
    glm_mat4_ins3(panel.model, model);
    glm_mat4_identity(proj);
    glm_ortho(0.0f, d->fbSize[0],  d->fbSize[1], 0.0f, 0.1f, 100.0f, proj);    
}

void terminate()
{
    glDeleteBuffers(1, &panel.vbo);
    glDeleteVertexArrays(1, &vao);
    shaderDestroy(simple);
    printf("Demo2d terminated\n");
}


void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
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
    glViewport(0, 0, (int)d->fbSize[0], (int)d->fbSize[1]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shaderUse(simple);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj[0]);
    glUniform1f(panel.alphaLoc, panel.alphas[panel.alpha]);
    glUniform3fv(colorLoc, 1, panel.colors[panel.color]);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

int main(void)
{
    d = demoCreate(init, terminate, update, render);
    demoRun();
    demoDestroy();

    return 0;
}