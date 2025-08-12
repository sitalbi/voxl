#version 450 core

in vec2 vUV;
in vec3 vWorldDir;  
out vec4 FragColor;

// Gradient params
uniform vec3 uHorizonColor = vec3(0.824, 0.953, 1.0);
uniform vec3 uZenithColor  = vec3(0.341, 0.824, 1.0);
uniform float uExponent    = 1.2;

// Sun params
uniform vec3  uSunDir;
uniform vec3  uSunColor     = vec3(1.0, 0.97, 0.50);
uniform float uSunAngularRadius = 0.02;
uniform float uSunSoftness      = 0.015;  
uniform float uSunIntensity     = 1.0;

void main()
{
    // Gradient 
    float t = pow(clamp(vUV.y, 0.0, 1.0), uExponent);
    vec3 col = mix(uHorizonColor, uZenithColor, t);

    // Sun  
    vec3 V = normalize(vWorldDir);
    float c = dot(V, normalize(uSunDir));
    float cosR   = cos(uSunAngularRadius);
    float cosRin = cos(max(uSunAngularRadius - uSunSoftness, 0.0));
    float sun    = smoothstep(cosR, cosRin, c); 

    col += uSunColor * (uSunIntensity * sun);

    FragColor = vec4(col, 1.0);
}
