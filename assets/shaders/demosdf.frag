#version 330 core
precision mediump float;

in vec2 uv0;

out vec4 FragColor;

uniform sampler2D mytexture;

const float smoothing = 1.0f/16.0f;

void main()
{
    float distance = texture(mytexture, uv0).r;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    FragColor = vec4(0.0f, 0.0f, 0.0f, alpha);
}
