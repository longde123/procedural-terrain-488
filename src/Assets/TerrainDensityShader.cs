#version 430

// first 3 components are the world coordinate
// 4th is the texture coordinate, should be 1, 2 or 4
uniform ivec4 block_index;

// Extra space on both sides that will be sampled by ambient occlusion.
uniform int block_padding;

uniform int octaves;
uniform float octaves_decay;
uniform float period;
uniform vec2 warp_params;

// One work group = 1 slice
// Note that 32x32 = 1024 which is the typical maximum work group size/block
// size for GPGPU languages, including CUDA, etc.
//
// NOTE: if you update this, update it on the CPU size too
layout(local_size_x = 16, local_size_y = 16, local_size_z = 4) in;

// One float per texture location.
layout(r32f, binding = 0) uniform image3D density_map;

#include "noise.h"

void main() {
    ivec3 img_coords = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 space_coords = img_coords - ivec3(block_padding);
    ivec3 block_dimensions = ivec3(gl_NumWorkGroups * gl_WorkGroupSize) - 2 * block_padding;

    float density = terrainDensity(
            vec3(space_coords * block_index.w) + block_index.xyz * (block_dimensions - 1),
            block_dimensions.y, period, octaves, octaves_decay);

    // Erosion, we want the lower-detail blocks the be slightly shaved off so that
    // we can render the higher-detail blocks with transparency on top of them with
    // z-layer conflicts.
    density -= (block_index.w - 1) * 0.02;

    imageStore(density_map, img_coords, vec4(density));
}
