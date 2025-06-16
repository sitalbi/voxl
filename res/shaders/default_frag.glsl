#version 450 core

out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D uAtlas;

void main()
{
	FragColor = texture(uAtlas, vTexCoord);
}