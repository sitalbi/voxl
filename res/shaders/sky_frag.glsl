#version 450 core

out vec4 FragColor;

in vec2 vUV;

// Tweak these at runtime for different times of day.
uniform vec3 uHorizonColor = vec3(0.824, 0.953, 1.0);
uniform vec3 uZenithColor  = vec3(0.341, 0.824, 1.0);
uniform float uExponent    = 1.2;  // curve sharpness; 1.0..2.5 is a good range

void main()
{
    float t = clamp(vUV.y, 0.0, 1.0);
    vec3 col = mix(uHorizonColor, uZenithColor, t);
    FragColor = vec4(col, 1.0);
}