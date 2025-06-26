#version 450 core

out vec4 FragColor;

in vec3 vTexCoord;

uniform sampler2DArray uTextureArray;
uniform bool uColorBlock;

void main()
{
	vec4 c = vec4(1.0,1.0,1.0,1.0);

	if (!uColorBlock) {
		c = texture(uTextureArray, vTexCoord);
	}

	//darken the color a bit
	c.rgb *= 0.8;

	FragColor = c;
}