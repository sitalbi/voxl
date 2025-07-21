﻿#version 450 core
// no inputs

void main() {
    // fullscreen triangle trick: 3 verts cover the whole screen
    const vec2 verts[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
}
