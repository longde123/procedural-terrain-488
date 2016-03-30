#version 430

layout(binding = 10) buffer Input0 {
    vec3 positions[];
} input_1;
layout(binding = 10) buffer Input1 {
    vec3 velocities[];
} input_2;
layout(binding = 11) buffer Input2 {
    vec3 colors[];
} input_3;

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

uniform int block_size;
uniform float period;
uniform vec2 warp_params;

#include "noise.h"

uint random(uint x)
{
    return hash(ivec3(x, 0, 0));
}

void main()
{
    uint index = gl_GlobalInvocationID.x;

    uint seed = random(index);
    int spread = 6;
    for (int i = 0; i < 1000; i++) {
        uint r1 = seed;
        seed = random(seed);
        uint r2 = seed;
        seed = random(seed);
        uint r3 = seed;
        seed = random(seed);

        vec3 coords = vec3(r1 % (block_size * 2 * spread) - block_size * spread,
                           float(r2 % 10) + block_size * 1.5,
                           r3 % (block_size * 2 * spread) - block_size * spread);

        // Three octaves is enough for collision detection.
        if (terrainDensity(coords, block_size, period, 3, 2.0) < -0.2) {
            // Air, we can place it there.
            input_1.positions[index] = coords / block_size;
            break;
        }
    }

    uint r1 = seed;
    seed = random(seed);
    uint r2 = seed;
    seed = random(seed);
    uint r3 = seed;
    seed = random(seed);
    input_2.velocities[index] = normalize(vec3(r1 % 256, r2 % 256, r3 % 256) - vec3(128.0));
    input_3.colors[index] = vec3(float(r1 % 256) / 255.0, float(r2 % 256) / 255.0, float(r3 % 256) / 255.0);
    input_3.colors[index] = vec3(1, 1, 1);
}
