#include "lgl/demo.h"
#include "lglui/demoui.h"

static Demo* d = NULL;
static Demoui* dui = NULL;

static GLuint quadVao;
static GLuint quadVbo;
static Shader* textureShader;
static mat4s model;
static int modelLoc;
static mat4s proj;
static int projLoc;
static int quadTexture;

static void render2D()
{
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, quadTexture);
    shaderUse(textureShader);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.raw[0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.raw[0]);
    glBindVertexArray(quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    //glDisable(GL_BLEND);
}

void init()
{
    printf("Demo texturing initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    dui = demouiInit();

    mat4s rotate = glms_rotate_y(GLMS_MAT4_IDENTITY, glm_rad(45.0f));
    model = glms_translate_z(GLMS_MAT4_IDENTITY, 20.0f);
    model = glms_mat4_mul(rotate, model);
    

    mat4s view;
    view = glms_lookat((vec3s){{0.0f, 0.0f, -40.0f}}, GLMS_VEC3_ZERO, GLMS_YUP);
    model = glms_mat4_mul(view, model);
    
    //glm_ortho(0.0f, d->fbSize[0], 0.0f, d->fbSize[1], 0.1f, 10.0f, proj);
    proj = glms_perspective(glm_rad(60.0f), d->fbSize.x / d->fbSize.y, 0.1f, 1000.0f);

    quadTexture = nvgCreateImage(dui->vg, "assets/textures/wall.jpg", 0);

    textureShader = shaderCreate("assets/shaders/demotext.vert", "assets/shaders/texture.frag");
    shaderUse(textureShader);
    modelLoc = glGetUniformLocation(textureShader->program, "model");
    projLoc = glGetUniformLocation(textureShader->program, "proj");
    glUseProgram(0);

    float quad[] = {
        0.0f, 0.0f,   0.0f,      0.0f, 0.0f,
        20.0f, 0.0f,  0.0f,      1.0f, 0.0f,
        0.0f, 20.0f,  0.0f,      0.0f, 1.0f,
        0.0f, 20.0f,  0.0f,      0.0f, 1.0f,
        20.0f, 0.0f,  0.0f,      1.0f, 0.0f,
        20.0f, 20.0f, 0.0f,      1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);
    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 30, quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void terminate()
{
    demouiTerminate();
    printf("Demo texturing terminated\n");
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
        render2D();
        break;
    case PASS_2D:
        break;
    case PASS_UI:
        {
            demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);
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
