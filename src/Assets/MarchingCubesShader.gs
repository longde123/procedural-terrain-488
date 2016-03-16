#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

//layout(r32f, binding = 0) uniform image3D density_map;
layout(binding = 0) uniform sampler3D density_map;

out vec3 vertex_color;

// The marching cubes algorithm consists of 256 cases of triangle configurations (each
// corner can be on or off).

// Tables by Ryan Geiss.

#define N 32

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

float density(vec3 coord)
{
    return texture(density_map, coord / N).x;
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

    vertex_color = density0123.xxx / 4.0 + 0.25;
    if (false) {
        // To visualize where the input points are.
        gl_Position = P * V * M * vec4(coords + vec3(0.0, 0.0, 0.0), 1.0);
        EmitVertex();
        gl_Position = P * V * M * vec4(coords + vec3(0.1, 0.0, 0.0), 1.0);
        EmitVertex();
        gl_Position = P * V * M * vec4(coords + vec3(0.0, 0.0, 0.1), 1.0);
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
            // Note that one side of the edge should always have a positive value
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

            gl_Position = P * V * M * vec4(v1, 1.0);
            EmitVertex();
            gl_Position = P * V * M * vec4(v2, 1.0);
            EmitVertex();
            gl_Position = P * V * M * vec4(v3, 1.0);
            EmitVertex();

            EndPrimitive();
        }
    }
}
