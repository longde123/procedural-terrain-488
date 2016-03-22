#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

layout(binding = 0) uniform sampler3D density_map;

uniform int block_size;
uniform bool short_range_ambient;
uniform bool long_range_ambient;
uniform float period;

out vec3 position;
out vec3 normal;
out float ambient_occlusion;

#include "noise.h"
#include "marching_cubes_common.h"

vec3 normalAtVertex(vec3 vertex)
{
    float d = 1.0;
    vec3 gradient = vec3(
        density(vertex + vec3(d, 0, 0)) - density(vertex - vec3(d, 0, 0)),
        density(vertex + vec3(0, d, 0)) - density(vertex - vec3(0, d, 0)),
        density(vertex + vec3(0, 0, d)) - density(vertex - vec3(0, 0, d)));
    return -normalize(gradient);
}

float ambientOcclusion(vec3 vertex)
{
    // TODO: Need to make sure ambient occlusion looks the same for all block sizes.
    float visibility = 0.0;
    for (int i = 0; i < 32; i++) {
        vec3 ray = random_rays[i];
        float ray_visibility = 1.0;

        // Short-range samples
        // Don't use multiplication! Adding is faster.
        // Start some (large) epsilon away.
        if (short_range_ambient) {
            vec3 short_ray = vertex + ray * 0.1;
            vec3 delta = ray / 4;
            for (int j = 0; j < 16; j++) {
                short_ray += delta;
                float d = density(short_ray);
                ray_visibility *= clamp(d * 8, 0.0, 1.0);
            }
        }

        // Long-range samples
        if (long_range_ambient) {
            for (int j = 0; j < 4; j++) {
                float distance = pow((j + 2) / 5.0, 1.8) * 40;
                float d = terrainDensity(vertex + distance * ray, block_size, period, 3);
                ray_visibility *= clamp(d * 0.5, 0.0, 1.0);
            }
        }

        visibility += ray_visibility;
    }

    return (1.0 - visibility / 32.0);
}

void createVertex(vec3 vertex)
{
    ambient_occlusion = ambientOcclusion(vertex);

    // Map vertices to range [0, 1]
    position = vertex / block_size;

    normal = normalAtVertex(vertex);
    EmitVertex();
}

void main() {
    vec3 coords = vec3(gl_in[0].gl_Position);

    // Use swizzling, avoid typing vector constructors for each corner.
    vec2 offset = vec2(0, 1);

    // More efficient to do vector operations.
    vec4 density0123;
    vec4 density4567;

    density0123.x = density(coords + offset.xxx);
    density0123.y = density(coords + offset.xyx);
    density0123.z = density(coords + offset.yyx);
    density0123.w = density(coords + offset.yxx);
    density4567.x = density(coords + offset.xxy);
    density4567.y = density(coords + offset.xyy);
    density4567.z = density(coords + offset.yyy);
    density4567.w = density(coords + offset.yxy);

    if (false) {
        ambient_occlusion = density0123.x / 4.0 + 0.25;
        normal = vec3(0);

        // To visualize where the input points are.
        position = coords + vec3(0.0, 0.0, 0.0);
        EmitVertex();
        position = coords + vec3(0.1, 0.0, 0.0);
        EmitVertex();
        position = coords + vec3(0.0, 0.0, 0.1);
        EmitVertex();
        EndPrimitive();
    } else {
        vec4 divider = vec4(0, 0, 0, 0);
        ivec4 ground0123 = ivec4(lessThan(divider, density0123));
        ivec4 ground4567 = ivec4(lessThan(divider, density4567));

        int case_index = (ground0123.x << 0) | (ground0123.y << 1) | (ground0123.z << 2) | (ground0123.w << 3) |
                         (ground4567.x << 4) | (ground4567.y << 5) | (ground4567.z << 6) | (ground4567.w << 7);
        int numpolys = case_to_numpolys[case_index];

        for (int i = 0; i < numpolys; i++) {
            ivec3 edge_index = edge_connect_list[case_index][i];

            // Want to place the vertex where the density is approximately zero.
            // note that one side of the edge should always have a positive value
            // and the other, a negative value.
            // So d1 * (1 - t) + d2 * t = 0 => t = d1 / (d1 - d2)
            // e.g. d1 = 0.1, d2 = -0.3 => t = 0.25

            vec3 d1 = vec3(density(coords + edge_start[edge_index.x]),
                           density(coords + edge_start[edge_index.y]),
                           density(coords + edge_start[edge_index.z]));
            vec3 d2 = vec3(density(coords + edge_end[edge_index.x]),
                           density(coords + edge_end[edge_index.y]),
                           density(coords + edge_end[edge_index.z]));
            vec3 t = d1 / (d1 - d2);

            vec3 v1 = edge_start[edge_index.x] + edge_dir[edge_index.x] * t.x;
            vec3 v2 = edge_start[edge_index.y] + edge_dir[edge_index.y] * t.y;
            vec3 v3 = edge_start[edge_index.z] + edge_dir[edge_index.z] * t.z;

            v1 += coords;
            v2 += coords;
            v3 += coords;

            createVertex(v1);
            createVertex(v2);
            createVertex(v3);

            EndPrimitive();
        }
    }
}

