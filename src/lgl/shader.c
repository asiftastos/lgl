#include "demo.h"
#include "assets.h"

static GLuint shaderCompile(const char* path, GLenum type)
{
    GLuint s = glCreateShader(type);
    FILE* f = fopen(path, "r");
    if(f == NULL)
    {
        printf("Cannot open shader %s", path);
        return 0;
    }
  
    char *text;
    long len;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    assert(len > 0);
    fseek(f, 0, SEEK_SET);
    text = calloc(1, len);
    assert(text != NULL);
    fread(text, 1, len, f);
    assert(strlen(text) > 0);
    fclose(f);
    
    glShaderSource(s, 1, (const char**)&text, NULL);
    glCompileShader(s);
    free(text);
    int compile_error = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compile_error);
    if (compile_error == GL_FALSE)
    {
        char log[1024];
        int i;
        glGetShaderInfoLog(s, 1024, &i, log);
        printf("Shader compile error: %s\n", log);
        return 0;
    }
    
    return s;
}

Shader* shaderCreate(const char* vspath, const char* fspath)
{
    Shader* s = (Shader*)calloc(1, sizeof(Shader));
    assert(s);
    s->vshader = shaderCompile(vspath, GL_VERTEX_SHADER);
    s->fshader = shaderCompile(fspath, GL_FRAGMENT_SHADER);

    s->program = glCreateProgram();
    glAttachShader(s->program, s->vshader);
    glAttachShader(s->program, s->fshader);
    glLinkProgram(s->program);
    int compile_error = 0;
    glGetProgramiv(s->program, GL_LINK_STATUS, &compile_error);
    if (compile_error == GL_FALSE)
    {
        char log[1024];
        int i;
        glGetProgramInfoLog(s->program, 1024, &i, log);
        printf("Shader link error: %s\n", log);
        return 0;
    }
    return s;
}

void shaderDestroy(Shader* s)
{
    if(s)
    {
        glDeleteProgram(s->program);
        free((void*)s);
        s = NULL;
    }
}

void shaderUse(Shader* s)
{
    glUseProgram(s->program);
}