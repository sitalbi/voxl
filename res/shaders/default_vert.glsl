#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTexCoord;
layout(location = 3) in float aAo;

out vec3 vTexCoord;
out float vAo;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
	vAo = aAo; 
	vTexCoord = aTexCoord;
	gl_Position = uProjection * uView * uModel * vec4(aPos,1.0);
}