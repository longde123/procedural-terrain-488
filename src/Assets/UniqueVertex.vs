#version 430

in uint z6_y6_x6_edge4_in;

uniform ivec4 block_index;

uniform bool short_range_ambient;
uniform bool long_range_ambient;
uniform float period;
uniform int octaves;
uniform float octaves_decay;
uniform vec2 warp_params;

out vec3 position;
out vec3 normal;
out float ambient_occlusion;

out vertexData
{
    vec3 position;
    vec3 normal;
    float ambient_occlusion;
} vertex_out;

#include "noise.h"
#include "marching_cubes_common.h"
#include "terrain_vertex_common.h"

void main() {
    uint edge_index = z6_y6_x6_edge4_in & 0xF;
    ivec3 coords = ivec3((z6_y6_x6_edge4_in >> 4) & 0x3F,
                         (z6_y6_x6_edge4_in >> 10) & 0x3F,
                         (z6_y6_x6_edge4_in >> 16) & 0x3F);

    // Want to place the vertex where the density is approximately zero.
    // note that one side of the edge should always have a positive value
    // and the other, a negative value.
    // So d1 * (1 - t) + d2 * t = 0 => t = d1 / (d1 - d2)
    // e.g. d1 = 0.1, d2 = -0.3 => t = 0.25

    float d1 = density(coords + edge_start[edge_index]);
    float d2 = density(coords + edge_end[edge_index]);
    float t = d1 / (d1 - d2);

    vec3 vertex_position = edge_start[edge_index] + edge_dir[edge_index] * t.x;
    vertex_position += coords;

    ambient_occlusion = ambientOcclusion(
        vertex_position,
        vertex_position * block_index.w + block_index.xyz * block_size,
        block_index.w,
        short_range_ambient, long_range_ambient);

    // Map vertices to range [0, 1]
    position = vertex_position / block_size;

    normal = normalAtVertex(vertex_position);
}
