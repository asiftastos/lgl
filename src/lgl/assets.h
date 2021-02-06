#ifndef ASSETS_HEADER_H
#define ASSETS_HEADER_H

typedef struct Shader
{
    GLuint program;
    GLuint vshader;
    GLuint fshader;
}Shader;

Shader* shaderCreate(const char* vspath, const char* fspath);
void shaderDestroy(Shader* s);
void shaderUse(Shader* s);

#endif