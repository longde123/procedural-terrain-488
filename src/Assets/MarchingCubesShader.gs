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

// The marching cubes algorithm consists of 256 cases of triangle configurations (each
// corner can be on or off).

// Tables by Ryan Geiss.

// Lookup table for many polygons for each of the 256 cases.
int case_to_numpolys[256] = {
    0, 1, 1, 2, 1, 2, 2, 3,  1, 2, 2, 3, 2, 3, 3, 2,  1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 2, 3, 4, 4, 3,  3, 4, 4, 3, 4, 5, 5, 2,
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,
    2, 3, 3, 4, 3, 4, 2, 3,  3, 4, 4, 5, 4, 5, 3, 2,  3, 4, 4, 3, 4, 5, 3, 2,  4, 5, 5, 4, 5, 2, 4, 1,
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 2, 4, 3, 4, 3, 5, 2,
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  3, 4, 4, 3, 4, 5, 5, 4,  4, 3, 5, 2, 5, 4, 2, 1,
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 2, 3, 3, 2,  3, 4, 4, 5, 4, 5, 5, 2,  4, 3, 5, 4, 3, 2, 4, 1,
    3, 4, 4, 5, 4, 5, 3, 4,  4, 5, 5, 2, 3, 4, 2, 1,  2, 3, 3, 2, 3, 4, 2, 1,  3, 2, 4, 1, 2, 1, 1, 0
};

vec3 edge_start[12] = {
    vec3(0, 0, 0), vec3(0, 1, 0), vec3(1, 0, 0), vec3(0, 0, 0),
    vec3(0, 0, 1), vec3(0, 1, 1), vec3(1, 0, 1), vec3(0, 0, 1),
    vec3(0, 0, 0), vec3(0, 1, 0), vec3(1, 1, 0), vec3(1, 0, 0)
};

vec3 edge_dir[12] = {
    vec3(0, 1, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(1, 0, 0),
    vec3(0, 1, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(1, 0, 0),
    vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1)
};

// Equal to edge_start + edge_dir
vec3 edge_end[12] = {
    vec3(0, 1, 0), vec3(1, 1, 0), vec3(1, 1, 0), vec3(1, 0, 0),
    vec3(0, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 0, 1),
    vec3(0, 0, 1), vec3(0, 1, 1), vec3(1, 1, 1), vec3(1, 0, 1)
};

vec3 edge_axis[12] = {
    vec3(1, 0, 0), vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 0, 0),
    vec3(1, 0, 0), vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 0, 0),
    vec3(2, 0, 0), vec3(2, 0, 0), vec3(2, 0, 0), vec3(2, 0, 0)
};

// Lookup table for the triangles in each case.
// Each case can have up to 5 triangles.
// Each triangle is defined by the index of the 3 edges in which its vertices are located.
ivec3 edge_connect_list[256][5] = {
    { ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  1,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  8,  3), ivec3( 9,  8,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  3), ivec3( 1,  2, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  2, 10), ivec3( 0,  2,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  8,  3), ivec3( 2, 10,  8), ivec3(10,  9,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3, 11,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0, 11,  2), ivec3( 8, 11,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  9,  0), ivec3( 2,  3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1, 11,  2), ivec3( 1,  9, 11), ivec3( 9,  8, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3, 10,  1), ivec3(11, 10,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0, 10,  1), ivec3( 0,  8, 10), ivec3( 8, 11, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  9,  0), ivec3( 3, 11,  9), ivec3(11, 10,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  8, 10), ivec3(10,  8, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  7,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  3,  0), ivec3( 7,  3,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  1,  9), ivec3( 8,  4,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  1,  9), ivec3( 4,  7,  1), ivec3( 7,  3,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 10), ivec3( 8,  4,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  4,  7), ivec3( 3,  0,  4), ivec3( 1,  2, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  2, 10), ivec3( 9,  0,  2), ivec3( 8,  4,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2, 10,  9), ivec3( 2,  9,  7), ivec3( 2,  7,  3), ivec3( 7,  9,  4), ivec3(-1, -1, -1) },
    { ivec3( 8,  4,  7), ivec3( 3, 11,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(11,  4,  7), ivec3(11,  2,  4), ivec3( 2,  0,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  0,  1), ivec3( 8,  4,  7), ivec3( 2,  3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  7, 11), ivec3( 9,  4, 11), ivec3( 9, 11,  2), ivec3( 9,  2,  1), ivec3(-1, -1, -1) },
    { ivec3( 3, 10,  1), ivec3( 3, 11, 10), ivec3( 7,  8,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1, 11, 10), ivec3( 1,  4, 11), ivec3( 1,  0,  4), ivec3( 7, 11,  4), ivec3(-1, -1, -1) },
    { ivec3( 4,  7,  8), ivec3( 9,  0, 11), ivec3( 9, 11, 10), ivec3(11,  0,  3), ivec3(-1, -1, -1) },
    { ivec3( 4,  7, 11), ivec3( 4, 11,  9), ivec3( 9, 11, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  4), ivec3( 0,  8,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  5,  4), ivec3( 1,  5,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  5,  4), ivec3( 8,  3,  5), ivec3( 3,  1,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 10), ivec3( 9,  5,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  0,  8), ivec3( 1,  2, 10), ivec3( 4,  9,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5,  2, 10), ivec3( 5,  4,  2), ivec3( 4,  0,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2, 10,  5), ivec3( 3,  2,  5), ivec3( 3,  5,  4), ivec3( 3,  4,  8), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  4), ivec3( 2,  3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0, 11,  2), ivec3( 0,  8, 11), ivec3( 4,  9,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  5,  4), ivec3( 0,  1,  5), ivec3( 2,  3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  1,  5), ivec3( 2,  5,  8), ivec3( 2,  8, 11), ivec3( 4,  8,  5), ivec3(-1, -1, -1) },
    { ivec3(10,  3, 11), ivec3(10,  1,  3), ivec3( 9,  5,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  9,  5), ivec3( 0,  8,  1), ivec3( 8, 10,  1), ivec3( 8, 11, 10), ivec3(-1, -1, -1) },
    { ivec3( 5,  4,  0), ivec3( 5,  0, 11), ivec3( 5, 11, 10), ivec3(11,  0,  3), ivec3(-1, -1, -1) },
    { ivec3( 5,  4,  8), ivec3( 5,  8, 10), ivec3(10,  8, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  7,  8), ivec3( 5,  7,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  3,  0), ivec3( 9,  5,  3), ivec3( 5,  7,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  7,  8), ivec3( 0,  1,  7), ivec3( 1,  5,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  5,  3), ivec3( 3,  5,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  7,  8), ivec3( 9,  5,  7), ivec3(10,  1,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  1,  2), ivec3( 9,  5,  0), ivec3( 5,  3,  0), ivec3( 5,  7,  3), ivec3(-1, -1, -1) },
    { ivec3( 8,  0,  2), ivec3( 8,  2,  5), ivec3( 8,  5,  7), ivec3(10,  5,  2), ivec3(-1, -1, -1) },
    { ivec3( 2, 10,  5), ivec3( 2,  5,  3), ivec3( 3,  5,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 7,  9,  5), ivec3( 7,  8,  9), ivec3( 3, 11,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  7), ivec3( 9,  7,  2), ivec3( 9,  2,  0), ivec3( 2,  7, 11), ivec3(-1, -1, -1) },
    { ivec3( 2,  3, 11), ivec3( 0,  1,  8), ivec3( 1,  7,  8), ivec3( 1,  5,  7), ivec3(-1, -1, -1) },
    { ivec3(11,  2,  1), ivec3(11,  1,  7), ivec3( 7,  1,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  8), ivec3( 8,  5,  7), ivec3(10,  1,  3), ivec3(10,  3, 11), ivec3(-1, -1, -1) },
    { ivec3( 5,  7,  0), ivec3( 5,  0,  9), ivec3( 7, 11,  0), ivec3( 1,  0, 10), ivec3(11, 10,  0) },
    { ivec3(11, 10,  0), ivec3(11,  0,  3), ivec3(10,  5,  0), ivec3( 8,  0,  7), ivec3( 5,  7,  0) },
    { ivec3(11, 10,  5), ivec3( 7, 11,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  6,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  3), ivec3( 5, 10,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  0,  1), ivec3( 5, 10,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  8,  3), ivec3( 1,  9,  8), ivec3( 5, 10,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  6,  5), ivec3( 2,  6,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  6,  5), ivec3( 1,  2,  6), ivec3( 3,  0,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  6,  5), ivec3( 9,  0,  6), ivec3( 0,  2,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5,  9,  8), ivec3( 5,  8,  2), ivec3( 5,  2,  6), ivec3( 3,  2,  8), ivec3(-1, -1, -1) },
    { ivec3( 2,  3, 11), ivec3(10,  6,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(11,  0,  8), ivec3(11,  2,  0), ivec3(10,  6,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  1,  9), ivec3( 2,  3, 11), ivec3( 5, 10,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5, 10,  6), ivec3( 1,  9,  2), ivec3( 9, 11,  2), ivec3( 9,  8, 11), ivec3(-1, -1, -1) },
    { ivec3( 6,  3, 11), ivec3( 6,  5,  3), ivec3( 5,  1,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8, 11), ivec3( 0, 11,  5), ivec3( 0,  5,  1), ivec3( 5, 11,  6), ivec3(-1, -1, -1) },
    { ivec3( 3, 11,  6), ivec3( 0,  3,  6), ivec3( 0,  6,  5), ivec3( 0,  5,  9), ivec3(-1, -1, -1) },
    { ivec3( 6,  5,  9), ivec3( 6,  9, 11), ivec3(11,  9,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5, 10,  6), ivec3( 4,  7,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  3,  0), ivec3( 4,  7,  3), ivec3( 6,  5, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  9,  0), ivec3( 5, 10,  6), ivec3( 8,  4,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  6,  5), ivec3( 1,  9,  7), ivec3( 1,  7,  3), ivec3( 7,  9,  4), ivec3(-1, -1, -1) },
    { ivec3( 6,  1,  2), ivec3( 6,  5,  1), ivec3( 4,  7,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2,  5), ivec3( 5,  2,  6), ivec3( 3,  0,  4), ivec3( 3,  4,  7), ivec3(-1, -1, -1) },
    { ivec3( 8,  4,  7), ivec3( 9,  0,  5), ivec3( 0,  6,  5), ivec3( 0,  2,  6), ivec3(-1, -1, -1) },
    { ivec3( 7,  3,  9), ivec3( 7,  9,  4), ivec3( 3,  2,  9), ivec3( 5,  9,  6), ivec3( 2,  6,  9) },
    { ivec3( 3, 11,  2), ivec3( 7,  8,  4), ivec3(10,  6,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5, 10,  6), ivec3( 4,  7,  2), ivec3( 4,  2,  0), ivec3( 2,  7, 11), ivec3(-1, -1, -1) },
    { ivec3( 0,  1,  9), ivec3( 4,  7,  8), ivec3( 2,  3, 11), ivec3( 5, 10,  6), ivec3(-1, -1, -1) },
    { ivec3( 9,  2,  1), ivec3( 9, 11,  2), ivec3( 9,  4, 11), ivec3( 7, 11,  4), ivec3( 5, 10,  6) },
    { ivec3( 8,  4,  7), ivec3( 3, 11,  5), ivec3( 3,  5,  1), ivec3( 5, 11,  6), ivec3(-1, -1, -1) },
    { ivec3( 5,  1, 11), ivec3( 5, 11,  6), ivec3( 1,  0, 11), ivec3( 7, 11,  4), ivec3( 0,  4, 11) },
    { ivec3( 0,  5,  9), ivec3( 0,  6,  5), ivec3( 0,  3,  6), ivec3(11,  6,  3), ivec3( 8,  4,  7) },
    { ivec3( 6,  5,  9), ivec3( 6,  9, 11), ivec3( 4,  7,  9), ivec3( 7, 11,  9), ivec3(-1, -1, -1) },
    { ivec3(10,  4,  9), ivec3( 6,  4, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4, 10,  6), ivec3( 4,  9, 10), ivec3( 0,  8,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  0,  1), ivec3(10,  6,  0), ivec3( 6,  4,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  3,  1), ivec3( 8,  1,  6), ivec3( 8,  6,  4), ivec3( 6,  1, 10), ivec3(-1, -1, -1) },
    { ivec3( 1,  4,  9), ivec3( 1,  2,  4), ivec3( 2,  6,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  0,  8), ivec3( 1,  2,  9), ivec3( 2,  4,  9), ivec3( 2,  6,  4), ivec3(-1, -1, -1) },
    { ivec3( 0,  2,  4), ivec3( 4,  2,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  3,  2), ivec3( 8,  2,  4), ivec3( 4,  2,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  4,  9), ivec3(10,  6,  4), ivec3(11,  2,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  2), ivec3( 2,  8, 11), ivec3( 4,  9, 10), ivec3( 4, 10,  6), ivec3(-1, -1, -1) },
    { ivec3( 3, 11,  2), ivec3( 0,  1,  6), ivec3( 0,  6,  4), ivec3( 6,  1, 10), ivec3(-1, -1, -1) },
    { ivec3( 6,  4,  1), ivec3( 6,  1, 10), ivec3( 4,  8,  1), ivec3( 2,  1, 11), ivec3( 8, 11,  1) },
    { ivec3( 9,  6,  4), ivec3( 9,  3,  6), ivec3( 9,  1,  3), ivec3(11,  6,  3), ivec3(-1, -1, -1) },
    { ivec3( 8, 11,  1), ivec3( 8,  1,  0), ivec3(11,  6,  1), ivec3( 9,  1,  4), ivec3( 6,  4,  1) },
    { ivec3( 3, 11,  6), ivec3( 3,  6,  0), ivec3( 0,  6,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 6,  4,  8), ivec3(11,  6,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 7, 10,  6), ivec3( 7,  8, 10), ivec3( 8,  9, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  7,  3), ivec3( 0, 10,  7), ivec3( 0,  9, 10), ivec3( 6,  7, 10), ivec3(-1, -1, -1) },
    { ivec3(10,  6,  7), ivec3( 1, 10,  7), ivec3( 1,  7,  8), ivec3( 1,  8,  0), ivec3(-1, -1, -1) },
    { ivec3(10,  6,  7), ivec3(10,  7,  1), ivec3( 1,  7,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2,  6), ivec3( 1,  6,  8), ivec3( 1,  8,  9), ivec3( 8,  6,  7), ivec3(-1, -1, -1) },
    { ivec3( 2,  6,  9), ivec3( 2,  9,  1), ivec3( 6,  7,  9), ivec3( 0,  9,  3), ivec3( 7,  3,  9) },
    { ivec3( 7,  8,  0), ivec3( 7,  0,  6), ivec3( 6,  0,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 7,  3,  2), ivec3( 6,  7,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  3, 11), ivec3(10,  6,  8), ivec3(10,  8,  9), ivec3( 8,  6,  7), ivec3(-1, -1, -1) },
    { ivec3( 2,  0,  7), ivec3( 2,  7, 11), ivec3( 0,  9,  7), ivec3( 6,  7, 10), ivec3( 9, 10,  7) },
    { ivec3( 1,  8,  0), ivec3( 1,  7,  8), ivec3( 1, 10,  7), ivec3( 6,  7, 10), ivec3( 2,  3, 11) },
    { ivec3(11,  2,  1), ivec3(11,  1,  7), ivec3(10,  6,  1), ivec3( 6,  7,  1), ivec3(-1, -1, -1) },
    { ivec3( 8,  9,  6), ivec3( 8,  6,  7), ivec3( 9,  1,  6), ivec3(11,  6,  3), ivec3( 1,  3,  6) },
    { ivec3( 0,  9,  1), ivec3(11,  6,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 7,  8,  0), ivec3( 7,  0,  6), ivec3( 3, 11,  0), ivec3(11,  6,  0), ivec3(-1, -1, -1) },
    { ivec3( 7, 11,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 7,  6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  0,  8), ivec3(11,  7,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  1,  9), ivec3(11,  7,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  1,  9), ivec3( 8,  3,  1), ivec3(11,  7,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  1,  2), ivec3( 6, 11,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 10), ivec3( 3,  0,  8), ivec3( 6, 11,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  9,  0), ivec3( 2, 10,  9), ivec3( 6, 11,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 6, 11,  7), ivec3( 2, 10,  3), ivec3(10,  8,  3), ivec3(10,  9,  8), ivec3(-1, -1, -1) },
    { ivec3( 7,  2,  3), ivec3( 6,  2,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 7,  0,  8), ivec3( 7,  6,  0), ivec3( 6,  2,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  7,  6), ivec3( 2,  3,  7), ivec3( 0,  1,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  6,  2), ivec3( 1,  8,  6), ivec3( 1,  9,  8), ivec3( 8,  7,  6), ivec3(-1, -1, -1) },
    { ivec3(10,  7,  6), ivec3(10,  1,  7), ivec3( 1,  3,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  7,  6), ivec3( 1,  7, 10), ivec3( 1,  8,  7), ivec3( 1,  0,  8), ivec3(-1, -1, -1) },
    { ivec3( 0,  3,  7), ivec3( 0,  7, 10), ivec3( 0, 10,  9), ivec3( 6, 10,  7), ivec3(-1, -1, -1) },
    { ivec3( 7,  6, 10), ivec3( 7, 10,  8), ivec3( 8, 10,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 6,  8,  4), ivec3(11,  8,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  6, 11), ivec3( 3,  0,  6), ivec3( 0,  4,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  6, 11), ivec3( 8,  4,  6), ivec3( 9,  0,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  4,  6), ivec3( 9,  6,  3), ivec3( 9,  3,  1), ivec3(11,  3,  6), ivec3(-1, -1, -1) },
    { ivec3( 6,  8,  4), ivec3( 6, 11,  8), ivec3( 2, 10,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 10), ivec3( 3,  0, 11), ivec3( 0,  6, 11), ivec3( 0,  4,  6), ivec3(-1, -1, -1) },
    { ivec3( 4, 11,  8), ivec3( 4,  6, 11), ivec3( 0,  2,  9), ivec3( 2, 10,  9), ivec3(-1, -1, -1) },
    { ivec3(10,  9,  3), ivec3(10,  3,  2), ivec3( 9,  4,  3), ivec3(11,  3,  6), ivec3( 4,  6,  3) },
    { ivec3( 8,  2,  3), ivec3( 8,  4,  2), ivec3( 4,  6,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  4,  2), ivec3( 4,  6,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  9,  0), ivec3( 2,  3,  4), ivec3( 2,  4,  6), ivec3( 4,  3,  8), ivec3(-1, -1, -1) },
    { ivec3( 1,  9,  4), ivec3( 1,  4,  2), ivec3( 2,  4,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  1,  3), ivec3( 8,  6,  1), ivec3( 8,  4,  6), ivec3( 6, 10,  1), ivec3(-1, -1, -1) },
    { ivec3(10,  1,  0), ivec3(10,  0,  6), ivec3( 6,  0,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  6,  3), ivec3( 4,  3,  8), ivec3( 6, 10,  3), ivec3( 0,  3,  9), ivec3(10,  9,  3) },
    { ivec3(10,  9,  4), ivec3( 6, 10,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  9,  5), ivec3( 7,  6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  3), ivec3( 4,  9,  5), ivec3(11,  7,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5,  0,  1), ivec3( 5,  4,  0), ivec3( 7,  6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(11,  7,  6), ivec3( 8,  3,  4), ivec3( 3,  5,  4), ivec3( 3,  1,  5), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  4), ivec3(10,  1,  2), ivec3( 7,  6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 6, 11,  7), ivec3( 1,  2, 10), ivec3( 0,  8,  3), ivec3( 4,  9,  5), ivec3(-1, -1, -1) },
    { ivec3( 7,  6, 11), ivec3( 5,  4, 10), ivec3( 4,  2, 10), ivec3( 4,  0,  2), ivec3(-1, -1, -1) },
    { ivec3( 3,  4,  8), ivec3( 3,  5,  4), ivec3( 3,  2,  5), ivec3(10,  5,  2), ivec3(11,  7,  6) },
    { ivec3( 7,  2,  3), ivec3( 7,  6,  2), ivec3( 5,  4,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  4), ivec3( 0,  8,  6), ivec3( 0,  6,  2), ivec3( 6,  8,  7), ivec3(-1, -1, -1) },
    { ivec3( 3,  6,  2), ivec3( 3,  7,  6), ivec3( 1,  5,  0), ivec3( 5,  4,  0), ivec3(-1, -1, -1) },
    { ivec3( 6,  2,  8), ivec3( 6,  8,  7), ivec3( 2,  1,  8), ivec3( 4,  8,  5), ivec3( 1,  5,  8) },
    { ivec3( 9,  5,  4), ivec3(10,  1,  6), ivec3( 1,  7,  6), ivec3( 1,  3,  7), ivec3(-1, -1, -1) },
    { ivec3( 1,  6, 10), ivec3( 1,  7,  6), ivec3( 1,  0,  7), ivec3( 8,  7,  0), ivec3( 9,  5,  4) },
    { ivec3( 4,  0, 10), ivec3( 4, 10,  5), ivec3( 0,  3, 10), ivec3( 6, 10,  7), ivec3( 3,  7, 10) },
    { ivec3( 7,  6, 10), ivec3( 7, 10,  8), ivec3( 5,  4, 10), ivec3( 4,  8, 10), ivec3(-1, -1, -1) },
    { ivec3( 6,  9,  5), ivec3( 6, 11,  9), ivec3(11,  8,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  6, 11), ivec3( 0,  6,  3), ivec3( 0,  5,  6), ivec3( 0,  9,  5), ivec3(-1, -1, -1) },
    { ivec3( 0, 11,  8), ivec3( 0,  5, 11), ivec3( 0,  1,  5), ivec3( 5,  6, 11), ivec3(-1, -1, -1) },
    { ivec3( 6, 11,  3), ivec3( 6,  3,  5), ivec3( 5,  3,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 10), ivec3( 9,  5, 11), ivec3( 9, 11,  8), ivec3(11,  5,  6), ivec3(-1, -1, -1) },
    { ivec3( 0, 11,  3), ivec3( 0,  6, 11), ivec3( 0,  9,  6), ivec3( 5,  6,  9), ivec3( 1,  2, 10) },
    { ivec3(11,  8,  5), ivec3(11,  5,  6), ivec3( 8,  0,  5), ivec3(10,  5,  2), ivec3( 0,  2,  5) },
    { ivec3( 6, 11,  3), ivec3( 6,  3,  5), ivec3( 2, 10,  3), ivec3(10,  5,  3), ivec3(-1, -1, -1) },
    { ivec3( 5,  8,  9), ivec3( 5,  2,  8), ivec3( 5,  6,  2), ivec3( 3,  8,  2), ivec3(-1, -1, -1) },
    { ivec3( 9,  5,  6), ivec3( 9,  6,  0), ivec3( 0,  6,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  5,  8), ivec3( 1,  8,  0), ivec3( 5,  6,  8), ivec3( 3,  8,  2), ivec3( 6,  2,  8) },
    { ivec3( 1,  5,  6), ivec3( 2,  1,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  3,  6), ivec3( 1,  6, 10), ivec3( 3,  8,  6), ivec3( 5,  6,  9), ivec3( 8,  9,  6) },
    { ivec3(10,  1,  0), ivec3(10,  0,  6), ivec3( 9,  5,  0), ivec3( 5,  6,  0), ivec3(-1, -1, -1) },
    { ivec3( 0,  3,  8), ivec3( 5,  6, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  5,  6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(11,  5, 10), ivec3( 7,  5, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(11,  5, 10), ivec3(11,  7,  5), ivec3( 8,  3,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5, 11,  7), ivec3( 5, 10, 11), ivec3( 1,  9,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(10,  7,  5), ivec3(10, 11,  7), ivec3( 9,  8,  1), ivec3( 8,  3,  1), ivec3(-1, -1, -1) },
    { ivec3(11,  1,  2), ivec3(11,  7,  1), ivec3( 7,  5,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  3), ivec3( 1,  2,  7), ivec3( 1,  7,  5), ivec3( 7,  2, 11), ivec3(-1, -1, -1) },
    { ivec3( 9,  7,  5), ivec3( 9,  2,  7), ivec3( 9,  0,  2), ivec3( 2, 11,  7), ivec3(-1, -1, -1) },
    { ivec3( 7,  5,  2), ivec3( 7,  2, 11), ivec3( 5,  9,  2), ivec3( 3,  2,  8), ivec3( 9,  8,  2) },
    { ivec3( 2,  5, 10), ivec3( 2,  3,  5), ivec3( 3,  7,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  2,  0), ivec3( 8,  5,  2), ivec3( 8,  7,  5), ivec3(10,  2,  5), ivec3(-1, -1, -1) },
    { ivec3( 9,  0,  1), ivec3( 5, 10,  3), ivec3( 5,  3,  7), ivec3( 3, 10,  2), ivec3(-1, -1, -1) },
    { ivec3( 9,  8,  2), ivec3( 9,  2,  1), ivec3( 8,  7,  2), ivec3(10,  2,  5), ivec3( 7,  5,  2) },
    { ivec3( 1,  3,  5), ivec3( 3,  7,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  7), ivec3( 0,  7,  1), ivec3( 1,  7,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  0,  3), ivec3( 9,  3,  5), ivec3( 5,  3,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9,  8,  7), ivec3( 5,  9,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5,  8,  4), ivec3( 5, 10,  8), ivec3(10, 11,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 5,  0,  4), ivec3( 5, 11,  0), ivec3( 5, 10, 11), ivec3(11,  3,  0), ivec3(-1, -1, -1) },
    { ivec3( 0,  1,  9), ivec3( 8,  4, 10), ivec3( 8, 10, 11), ivec3(10,  4,  5), ivec3(-1, -1, -1) },
    { ivec3(10, 11,  4), ivec3(10,  4,  5), ivec3(11,  3,  4), ivec3( 9,  4,  1), ivec3( 3,  1,  4) },
    { ivec3( 2,  5,  1), ivec3( 2,  8,  5), ivec3( 2, 11,  8), ivec3( 4,  5,  8), ivec3(-1, -1, -1) },
    { ivec3( 0,  4, 11), ivec3( 0, 11,  3), ivec3( 4,  5, 11), ivec3( 2, 11,  1), ivec3( 5,  1, 11) },
    { ivec3( 0,  2,  5), ivec3( 0,  5,  9), ivec3( 2, 11,  5), ivec3( 4,  5,  8), ivec3(11,  8,  5) },
    { ivec3( 9,  4,  5), ivec3( 2, 11,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  5, 10), ivec3( 3,  5,  2), ivec3( 3,  4,  5), ivec3( 3,  8,  4), ivec3(-1, -1, -1) },
    { ivec3( 5, 10,  2), ivec3( 5,  2,  4), ivec3( 4,  2,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3, 10,  2), ivec3( 3,  5, 10), ivec3( 3,  8,  5), ivec3( 4,  5,  8), ivec3( 0,  1,  9) },
    { ivec3( 5, 10,  2), ivec3( 5,  2,  4), ivec3( 1,  9,  2), ivec3( 9,  4,  2), ivec3(-1, -1, -1) },
    { ivec3( 8,  4,  5), ivec3( 8,  5,  3), ivec3( 3,  5,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  4,  5), ivec3( 1,  0,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 8,  4,  5), ivec3( 8,  5,  3), ivec3( 9,  0,  5), ivec3( 0,  3,  5), ivec3(-1, -1, -1) },
    { ivec3( 9,  4,  5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4, 11,  7), ivec3( 4,  9, 11), ivec3( 9, 10, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  8,  3), ivec3( 4,  9,  7), ivec3( 9, 11,  7), ivec3( 9, 10, 11), ivec3(-1, -1, -1) },
    { ivec3( 1, 10, 11), ivec3( 1, 11,  4), ivec3( 1,  4,  0), ivec3( 7,  4, 11), ivec3(-1, -1, -1) },
    { ivec3( 3,  1,  4), ivec3( 3,  4,  8), ivec3( 1, 10,  4), ivec3( 7,  4, 11), ivec3(10, 11,  4) },
    { ivec3( 4, 11,  7), ivec3( 9, 11,  4), ivec3( 9,  2, 11), ivec3( 9,  1,  2), ivec3(-1, -1, -1) },
    { ivec3( 9,  7,  4), ivec3( 9, 11,  7), ivec3( 9,  1, 11), ivec3( 2, 11,  1), ivec3( 0,  8,  3) },
    { ivec3(11,  7,  4), ivec3(11,  4,  2), ivec3( 2,  4,  0), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(11,  7,  4), ivec3(11,  4,  2), ivec3( 8,  3,  4), ivec3( 3,  2,  4), ivec3(-1, -1, -1) },
    { ivec3( 2,  9, 10), ivec3( 2,  7,  9), ivec3( 2,  3,  7), ivec3( 7,  4,  9), ivec3(-1, -1, -1) },
    { ivec3( 9, 10,  7), ivec3( 9,  7,  4), ivec3(10,  2,  7), ivec3( 8,  7,  0), ivec3( 2,  0,  7) },
    { ivec3( 3,  7, 10), ivec3( 3, 10,  2), ivec3( 7,  4, 10), ivec3( 1, 10,  0), ivec3( 4,  0, 10) },
    { ivec3( 1, 10,  2), ivec3( 8,  7,  4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  9,  1), ivec3( 4,  1,  7), ivec3( 7,  1,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  9,  1), ivec3( 4,  1,  7), ivec3( 0,  8,  1), ivec3( 8,  7,  1), ivec3(-1, -1, -1) },
    { ivec3( 4,  0,  3), ivec3( 7,  4,  3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 4,  8,  7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9, 10,  8), ivec3(10, 11,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  0,  9), ivec3( 3,  9, 11), ivec3(11,  9, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  1, 10), ivec3( 0, 10,  8), ivec3( 8, 10, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  1, 10), ivec3(11,  3, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  2, 11), ivec3( 1, 11,  9), ivec3( 9, 11,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  0,  9), ivec3( 3,  9, 11), ivec3( 1,  2,  9), ivec3( 2, 11,  9), ivec3(-1, -1, -1) },
    { ivec3( 0,  2, 11), ivec3( 8,  0, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 3,  2, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  3,  8), ivec3( 2,  8, 10), ivec3(10,  8,  9), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 9, 10,  2), ivec3( 0,  9,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 2,  3,  8), ivec3( 2,  8, 10), ivec3( 0,  1,  8), ivec3( 1, 10,  8), ivec3(-1, -1, -1) },
    { ivec3( 1, 10,  2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 1,  3,  8), ivec3( 9,  1,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  9,  1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3( 0,  3,  8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
    { ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) },
};

// 32 random rays on sphere with Poisson distribution, table from Ryan Geiss.
vec3 random_rays[32] = {
    vec3( 0.286582,  0.257763, -0.922729),
    vec3(-0.171812, -0.888079,  0.426375),
    vec3( 0.440764, -0.502089, -0.744066),
    vec3(-0.841007, -0.428818, -0.329882),
    vec3(-0.380213, -0.588038, -0.713898),
    vec3(-0.055393, -0.207160, -0.976738),
    vec3(-0.901510, -0.077811,  0.425706),
    vec3(-0.974593,  0.123830, -0.186643),
    vec3( 0.208042, -0.524280,  0.825741),
    vec3( 0.258429, -0.898570, -0.354663),
    vec3(-0.262118,  0.574475, -0.775418),
    vec3( 0.735212,  0.551820,  0.393646),
    vec3( 0.828700, -0.523923, -0.196877),
    vec3( 0.788742,  0.005727, -0.614698),
    vec3(-0.696885,  0.649338, -0.304486),
    vec3(-0.625313,  0.082413, -0.776010),
    vec3( 0.358696,  0.928723,  0.093864),
    vec3( 0.188264,  0.628978,  0.754283),
    vec3(-0.495193,  0.294596,  0.817311),
    vec3( 0.818889,  0.508670, -0.265851),
    vec3( 0.027189,  0.057757,  0.997960),
    vec3(-0.188421,  0.961802, -0.198582),
    vec3( 0.995439,  0.019982,  0.093282),
    vec3(-0.315254, -0.925345, -0.210596),
    vec3( 0.411992, -0.877706,  0.244733),
    vec3( 0.625857,  0.080059,  0.775818),
    vec3(-0.243839,  0.866185,  0.436194),
    vec3(-0.725464, -0.643645,  0.243768),
    vec3( 0.766785, -0.430702,  0.475959),
    vec3(-0.446376, -0.391664,  0.804580),
    vec3(-0.761557,  0.562508,  0.321895),
    vec3( 0.344460,  0.753223, -0.560359)
};

#include "noise.h"

float density(vec3 coord)
{
    return texture(density_map, coord / block_size).x;
}

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
    position = vertex;
    normal = normalAtVertex(vertex);
    EmitVertex();
}

void main() {
    vec3 coords = vec3(gl_in[0].gl_Position);

    // Use swizzling, avoid typing vector constructors for each corner.
    vec2 offset = vec2(0, 1);

    //ivec2 dimensions = imageSize(density_map);

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
            // block_sizeote that one side of the edge should always have a positive value
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

