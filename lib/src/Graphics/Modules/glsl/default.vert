#version 460 core

layout(location = 0) out vec3 outColor;

static const vec3 positions[3] = vec3[]
(
    vec3(0., -.5, 0.),
    vec3(-.5, .5 ,0.),
    vec3(.5, .5, 0.)
);

static const vec3 colors[3] = vec3[]
(
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    outColor = colors[gl_VertexIndex];
}