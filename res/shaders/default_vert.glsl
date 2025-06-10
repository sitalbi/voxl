#version 450 core

layout(location = 0) in vec4 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
	gl_Position = uProjection * uView * uModel * aPos;
}