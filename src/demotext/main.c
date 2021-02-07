#include "lgl/demo.h"
#include "stb_ds.h"
#include "stb_rect_pack.h"
#include "stb_truetype.h"

typedef struct VertexTexture
{
    vec3 vertex;
    vec2 texCoord;
}VertexTexture;

typedef struct TextElem
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    char* text;
    vec2 position;
    bool changed;
    int indexCount;
}TextElem;


static Demo* d = NULL;
static stbtt_packedchar* charinfo = NULL;
static uint32_t firstChar = ' ';
static uint32_t charCount = '~' - ' ';
static GLuint fontTextureAtlas;
static TextElem debugText;
static Shader* textsh;
static mat4 model;
static int modelLoc;
static mat4 proj;
static int projLoc;


static void updateText()
{
    int textlength = strlen(debugText.text);
    float xpos = 10.0f; //x position for where to put text in the xaxis
    float ypos = -10.0f; //y position for where to put text in the yaxis, positive is minus down
    VertexTexture* vertbuffer = NULL;
    uint16_t* indices = NULL;
    
    for (int i = 0; i < textlength; i++)
    {
        stbtt_aligned_quad quad;
        stbtt_GetPackedQuad(charinfo, 1024, 1024, debugText.text[i] - firstChar, 
                            &xpos, &ypos, &quad, 1);

        const float xmin = quad.x0; //left
        const float xmax = quad.x1; //right
        const float ymin = -quad.y0; //top
        const float ymax = -quad.y1; //bottom
        
        VertexTexture vt1 = {{xmin, ymin, -1.0f}, {quad.s0, quad.t1}};
        arrput(vertbuffer, vt1);
        VertexTexture vt2 = {{xmin, ymax, -1.0f}, {quad.s0, quad.t0}};
        arrput(vertbuffer, vt2);
        VertexTexture vt3 = {{xmax, ymax, -1.0f}, {quad.s1, quad.t0}};
        arrput(vertbuffer, vt3);
        VertexTexture vt4 = {{xmax, ymin, -1.0f}, {quad.s1, quad.t1}};
        arrput(vertbuffer, vt4);

        uint16_t lastIndex = arrlen(vertbuffer) - 4;
        arrput(indices, lastIndex);
        arrput(indices, lastIndex + 2);
        arrput(indices, lastIndex + 1);
        arrput(indices, lastIndex);
        arrput(indices, lastIndex + 3);
        arrput(indices, lastIndex + 2);
    }

    glGenVertexArrays(1, &debugText.vao);
    glBindVertexArray(debugText.vao);
    glGenBuffers(1, &debugText.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, debugText.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTexture) * arrlen(vertbuffer), vertbuffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &debugText.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugText.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * arrlen(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    debugText.indexCount = arrlen(indices);

    arrfree(vertbuffer);
    arrfree(indices);
}

static void drawText()
{
    shaderUse(textsh);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj[0]);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model[0]);
    glBindTexture(GL_TEXTURE_2D, fontTextureAtlas);
    glBindVertexArray(debugText.vao);
    glDrawElements(GL_TRIANGLES, debugText.indexCount, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);
}

//==============================================================================
//          SDF
//==============================================================================
typedef struct
{
    float advance;
    signed char xoff;
    signed char yoff;
    unsigned int w,h;
    unsigned char *data;
} fontchar;

typedef struct AtlasRect
{
    float x0;
    float y0;
    float x1;
    float y1;
}AtlasRect;


float sdf_size = 48.0;          // the larger this is, the better large font sizes look
static stbtt_fontinfo font;
static float scale;
fontchar fdata[128];
TextElem sdfText;
Shader* sdfShader;
GLuint sdfTexture;
int texwidth;
int texheight;
static AtlasRect charInfoRects[128];

static void loadSDF()
{
    unsigned char* buffer = NULL;
    buffer = (unsigned char*)calloc(24<<20, sizeof(unsigned char));
    fread(buffer, 1, 1000000, fopen("assets/fonts/Roboto-Regular.ttf", "rb"));

    stbtt_InitFont(&font, buffer, 0);
    scale = stbtt_ScaleForPixelHeight(&font, sdf_size);
    printf("Font scale: %f\n", scale);

    texwidth = 0;
    for (int ch=32; ch < 127; ++ch) {
      fontchar fc;
      int xoff,yoff,w,h, advance;
      fc.data = stbtt_GetCodepointSDF(&font, scale, ch, 3, 128.0f, 48.0f, &w, &h, &xoff, &yoff);
      fc.xoff = xoff;
      fc.yoff = yoff;
      fc.w = w;
      fc.h = h;
      stbtt_GetCodepointHMetrics(&font, ch, &advance, NULL);
      fc.advance = advance * scale;
      fdata[ch] = fc;
      texwidth += w;
    }

    free((void*)buffer);

    texheight = sdf_size;
    int texsize = texwidth*texheight;
    unsigned char* teximage = (unsigned char*)calloc(texsize, sizeof(unsigned char));
    memset(teximage, 0, texsize);
    int xpos = 0;
    for(int i = 32; i < 127; ++i)
    {
        fontchar fc = fdata[i];
        for(unsigned int y = 0; y < fc.h; y++)
        {
            for (unsigned int x = 0; x < fc.w; x++)
            {
                unsigned char val = fc.data[(y*fc.w) + x];
                int index = y*texwidth + (xpos + x);
                teximage[index] = val;
            }
        }
        AtlasRect rect;
        rect.x0 = (float)xpos / (float)texwidth;
        rect.y0 = 0.0f;
        rect.x1 = (float)(xpos + fc.w) / (float)texwidth;
        rect.y1 = 1.0f;
        
        charInfoRects[i] = rect;

        xpos += fc.w;
    }
    
    
    glGenTextures(1, &sdfTexture);
    glBindTexture(GL_TEXTURE_2D, sdfTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texwidth, texheight, 0, GL_RED, GL_UNSIGNED_BYTE, teximage);
    glGenerateMipmap(GL_TEXTURE_2D);
    free((void*)teximage);
}

static void initSDF()
{
    VertexTexture* vertbuffer = NULL;
    uint16_t* indices = NULL;
    float xpos = 5.0f;
    float ypos = 60.0f;

    int l = strlen(sdfText.text);
    for (int i = 0; i < l; i++)
    {
        int c = sdfText.text[i];
        AtlasRect ar = charInfoRects[c];
        fontchar fc = fdata[c];

        //printf("Atlasrect [%c]: %d, %d, %d, [%d,%d]\n", c, fc.xoff, fc.yoff, yoff, fc.w, fc.h);
        
        int yoff = fc.h + fc.yoff;
        float xmin = xpos;
        float xmax = xpos + (float)fc.w;
        float ymin = ypos - (float)(fc.h - yoff); //reversed, from top-left to bottom-left
        float ymax = ypos;
        VertexTexture vt1 = {{xmin, ymin, -1.0f}, {ar.x0, ar.y0}};
        arrput(vertbuffer, vt1);
        VertexTexture vt2 = {{xmin, ymax, -1.0f}, {ar.x0, ar.y1}};
        arrput(vertbuffer, vt2);
        VertexTexture vt3 = {{xmax, ymax, -1.0f}, {ar.x1, ar.y1}};
        arrput(vertbuffer, vt3);
        VertexTexture vt4 = {{xmax, ymin, -1.0f}, {ar.x1, ar.y0}};
        arrput(vertbuffer, vt4);

        uint16_t lastIndex = arrlen(vertbuffer) - 4;
        arrput(indices, lastIndex);
        arrput(indices, lastIndex + 2);
        arrput(indices, lastIndex + 1);
        arrput(indices, lastIndex);
        arrput(indices, lastIndex + 3);
        arrput(indices, lastIndex + 2);

        xpos += fc.advance;
    }
    
    
    glGenVertexArrays(1, &sdfText.vao);
    glBindVertexArray(sdfText.vao);
    glGenBuffers(1, &sdfText.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sdfText.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTexture) * arrlen(vertbuffer), vertbuffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &sdfText.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sdfText.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * arrlen(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    sdfText.indexCount = arrlen(indices);

    arrfree(vertbuffer);
    arrfree(indices);
}

static void drawSDF()
{
    shaderUse(sdfShader);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj[0]);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model[0]);
    glBindTexture(GL_TEXTURE_2D, sdfTexture);
    glBindVertexArray(sdfText.vao);
    glDrawElements(GL_TRIANGLES, sdfText.indexCount, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);
}

//===============================================================================
//              DEMO
// ==============================================================================
void demoInit()
{
    printf("Demotext initialized\n");
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    
    unsigned char* buffer = NULL;
    buffer = (unsigned char*)calloc(24<<20, sizeof(unsigned char));
    fread(buffer, 1, 1000000, fopen("assets/fonts/Roboto-Regular.ttf", "rb"));

    stbtt_pack_context packContext;
    uint8_t* atlasdata = (uint8_t*)calloc(1024 * 1024, sizeof(uint8_t));
    if(!stbtt_PackBegin(&packContext, atlasdata, 1024, 1024, 0, 1, NULL))
        printf("Failed to initialize font\n");
    
    charinfo = (stbtt_packedchar*)calloc(charCount, sizeof(stbtt_packedchar));
    stbtt_PackSetOversampling(&packContext, 2, 2); //better quality
    if(!stbtt_PackFontRange(&packContext, buffer, 0, 40, firstChar, charCount, charinfo))
        printf("Failed to pack ranges to atlas\n");
    
    stbtt_PackEnd(&packContext);
    free(buffer);

    glGenTextures(1, &fontTextureAtlas);
    glBindTexture(GL_TEXTURE_2D, fontTextureAtlas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1024, 1024, 0, GL_ALPHA, GL_UNSIGNED_BYTE, atlasdata);
    glGenerateMipmap(GL_TEXTURE_2D);

    free((void*)atlasdata);

    debugText.text = "Kostas";
    updateText();

    textsh = shaderCreate("assets/shaders/demotext.vert", "assets/shaders/demotext.frag");
    shaderUse(textsh);
    modelLoc = glGetUniformLocation(textsh->program, "model");
    projLoc = glGetUniformLocation(textsh->program, "proj");
    glm_mat4_identity(model);
    glm_scale(model, (vec3){1.0f, 1.0f, 1.0f});
    //glm_translate(model, (vec3){10.0f, 10.0f, 0.0f});
    glm_mat4_identity(proj);
    glm_ortho(0.0f, d->fbSize[0], d->fbSize[1], 0.0f, 0.1f, 100.0f, proj);

    loadSDF();
    sdfText.text = "Konstantinos";
    initSDF();
    sdfShader = shaderCreate("assets/shaders/demotext.vert", "assets/shaders/demosdf.frag");
    shaderUse(sdfShader);
}

void demoTerminate()
{
    glDeleteBuffers(1, &debugText.vbo);
    glDeleteBuffers(1, &debugText.ebo);
    glDeleteVertexArrays(1, &debugText.vao);
    shaderDestroy(textsh);
    glDeleteTextures(1, &fontTextureAtlas);
    if(charinfo)
        free((void*)charinfo);
    printf("Demotext terminated\n");
}

void demoUpdate()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(debugText.changed)
        updateText();
}

void demoRender()
{
    //needed otherwise draws all the rectangle around the letter.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //drawText();

    drawSDF();
}

int main(void)
{
    d = demoCreate(demoInit, demoTerminate, demoUpdate, demoRender);
    demoRun();
    demoDestroy();
    return 0;
}