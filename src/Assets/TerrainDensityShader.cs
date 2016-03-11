#version 430

// One work group = 1 slice
// Note that 32x32 = 1024 which is the typical maximum work group size/block
// size for GPGPU languages, including CUDA, etc.
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// One float per texture location.
layout(r32f) uniform image3D density_map;

void main() {
    ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 block_dimensions = ivec3(gl_NumWorkGroups * gl_WorkGroupSize);

    // Air is negative, ground is positive.
    // Generate a gradient from [1 to -1]
    float height_gradient = 1.0 - (float(coords.y) / block_dimensions.y) * 2;

    imageStore(density_map, coords, vec4(height_gradient));
}
