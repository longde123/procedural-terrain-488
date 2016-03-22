#version 430

uniform float block_size;
uniform float period;

in vertexData
{
    vec3 position;
} vertex_in;

out vec4 fragColor;

#include "noise.h"

void main() {
    vec3 coords = vertex_in.position;

    float density = terrainDensity(coords * block_size, block_size, period, 8);

    fragColor = vec4(vec3(0, 0, max(density, 0)) + vec3(max(-density, 0)), 1.0);
}
