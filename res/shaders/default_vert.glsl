#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTexCoord;
layout(location = 3) in float aAo;

out vec3 vTexCoord;
out float vAo;
out float vFogDepth;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
	vAo = aAo; 
	vTexCoord = aTexCoord;
	vec4 viewPos = uView * uModel * vec4(aPos, 1.0);
    vFogDepth    = -viewPos.z;

    gl_Position = uProjection * viewPos;
}