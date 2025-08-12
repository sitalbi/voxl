#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aUV;

out vec2 vUV;
out vec3 vWorldDir;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    vUV = vec2(aUV);
    
    vec4 viewPos = uView * vec4(aPos, 1.0);

    vec4 pos  = uProjection * viewPos;
    gl_Position = pos;
    gl_Position.z = pos.w; // ensure depth is correct for skybox


    // World-space view direction for the sun
    vec3 dirView  = normalize(viewPos.xyz);
    mat3 R = mat3(uView); 
    vWorldDir = transpose(R) * dirView; 
}  