#version 450 core

out vec4 FragColor;

in vec3 vTexCoord;
in float vAo;
in float vFogDepth;

uniform sampler2DArray uTextureArray;
uniform bool uColorBlock;

uniform float uLightIntensity;

uniform vec3   uFogColor;  // e.g. skyÅ]blue
uniform float  uFogStart;  // distance where fog begins
uniform float  uFogEnd;    // distance where fog is opaque


void main()
{
	vec4 c = vec4(1.0,1.0,1.0,1.0);

	if (!uColorBlock) {
		c = texture(uTextureArray, vTexCoord);

		c.rgb *= vAo;
	}

	// Apply alpha test with a=0.5, 
	// taking into account the smaller mip levels averaged pixels alpha values
	if(c.a < 0.5) {
		discard;
	}
	// Apply light intensity
	c.rgb *= uLightIntensity;

	// Fog
	float fogFactor = clamp((uFogEnd - vFogDepth) /
                            (uFogEnd - uFogStart), 0.0, 1.0);

    // fade both color and alpha
    c.rgb = mix(uFogColor, c.rgb, fogFactor);
    c.a   = c.a * fogFactor;  
 
    c.rgb = mix(uFogColor, c.rgb, fogFactor);


	FragColor = c;
}