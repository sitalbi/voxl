#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aUV;

out vec2 vUV;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    vUV = vec2(aUV);
    
    vec4 viewPos = uView * vec4(aPos, 1.0);

    vec4 pos  = uProjection * viewPos;
    gl_Position = pos;
    gl_Position.z = pos.w; // ensure depth is correct for skybox
}  