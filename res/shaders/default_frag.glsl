#version 450 core

out vec4 FragColor;

in vec3 vTexCoord;
in float vAo;

uniform sampler2DArray uTextureArray;
uniform bool uColorBlock;

void main()
{
	vec4 c = vec4(1.0,1.0,1.0,1.0);

	if (!uColorBlock) {
		c = texture(uTextureArray, vTexCoord);

		c.rgb *= vAo;
	}

	FragColor = c;
}