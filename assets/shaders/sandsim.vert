#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aCol;

uniform mat4 model;
uniform mat4 proj;

out vec4 color;

void main()
{
    gl_Position = proj * model * vec4(aPos, 1.0f);
    color = aCol;
}
