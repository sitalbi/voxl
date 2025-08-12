#version 450 core

in vec2 vUV;          // already in your shader
in vec3 vWorldDir;    // new: from VS (world-space view direction)
out vec4 FragColor;

uniform vec3 uHorizonColor = vec3(0.824, 0.953, 1.0);
uniform vec3 uZenithColor  = vec3(0.341, 0.824, 1.0);
uniform float uExponent    = 1.2;

// Sun params
uniform vec3  uSunDir;              // world-space, normalized
uniform vec3  uSunColor     = vec3(1.0, 0.97, 0.50);
uniform float uSunAngularRadius = 0.02;   // radians; stylized (real ~0.00465)
uniform float uSunSoftness      = 0.015;  // edge feather (radians)
uniform float uSunIntensity     = 1.0;

void main()
{
    // Screen-space gradient (top to bottom)
    float t = pow(clamp(vUV.y, 0.0, 1.0), uExponent);
    vec3 col = mix(uHorizonColor, uZenithColor, t);

    // Sun disc (dot only; no acos)
    vec3 V = normalize(vWorldDir);
    float c = dot(V, normalize(uSunDir));         // cos(angle between view ray & sun)
    float cosR   = cos(uSunAngularRadius);
    float cosRin = cos(max(uSunAngularRadius - uSunSoftness, 0.0));
    float sun    = smoothstep(cosR, cosRin, c);   // 0 outside -> 1 inside

    col += uSunColor * (uSunIntensity * sun);

    FragColor = vec4(col, 1.0);
}
