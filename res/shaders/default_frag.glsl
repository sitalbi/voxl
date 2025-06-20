#version 450 core

out vec4 FragColor;

in vec3 vTexCoord;

uniform sampler2DArray uTextureArray;

void main()
{
	FragColor = texture(uTextureArray, vTexCoord);
}