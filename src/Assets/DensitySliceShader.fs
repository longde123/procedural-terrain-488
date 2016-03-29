#version 430

// first 3 components are the world coordinate
// 4th is the block size in world space, should be 1, 2 or 4
uniform ivec4 block_index;

uniform float block_size;
uniform float period;
uniform int octaves;
uniform float octaves_decay;

in vertexData
{
    vec3 position;
} vertex_in;

out vec4 fragColor;

#include "noise.h"

void main() {
    vec3 coords = vertex_in.position;

    float density = terrainDensity((coords * block_index.w + block_index.xyz) * block_size,
                                   block_size, period, octaves, octaves_decay);

    float transition_width = 0.02;

    if (density >= 0.0 && density < transition_width) {
        fragColor = vec4(vec3(1 - density / transition_width, 0, max(density, 0)), 0.8);
    } else if (density < 0.0 && density > -transition_width) {
        fragColor = vec4(vec3(1 + density / transition_width, 0, max(-density, 0)), 0.8);
    } else {
        fragColor = vec4(vec3(0, 0, max(density, 0)) + vec3(max(-density, 0)), 0.8);
    }
}
