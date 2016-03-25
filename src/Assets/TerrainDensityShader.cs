#version 430

// first 3 components are the world coordinate
// 4th is the texture coordinate, should be 1, 2 or 4
uniform ivec4 block_index;

uniform float period;

// One work group = 1 slice
// Note that 32x32 = 1024 which is the typical maximum work group size/block
// size for GPGPU languages, including CUDA, etc.
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// One float per texture location.
layout(r32f, binding = 0) uniform image3D density_map;

#include "noise.h"

void main() {
    ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 block_dimensions = ivec3(gl_NumWorkGroups * gl_WorkGroupSize);

    float density = terrainDensity(
            vec3(coords * block_index.w) + block_index.xyz * (block_dimensions - 1),
            block_dimensions.y, period, 3);

    //imageStore(density_map, coords, mod(vec4(frequency), 1.0));
    //imageStore(density_map, coords, vec4(perlinNoise(coords, frequency)));
    imageStore(density_map, coords, vec4(density));
}
