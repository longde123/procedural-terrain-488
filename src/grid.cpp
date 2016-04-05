#include "grid.hpp"

#include <algorithm>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
Grid::Grid(size_t resolution)
: resolution(resolution)
{
}

void Grid::init(ShaderProgram& shaderProgram, mat4 transform)
{
    size_t vertex_count = 3 * resolution * resolution;

    float *verts = new float[vertex_count];

    // Need <=. A grid of resolution 1x1
    for (int y = 0; y < resolution; y++) {
        for (int x = 0; x < resolution; x++) {
            vec3 point = vec3(transform * vec4(x, 0, y, 1));

            int idx = x + y * resolution;
            verts[idx * 3 + 0] = point.x;
            verts[idx * 3 + 1] = point.y;
            verts[idx * 3 + 2] = point.z;
        }
    }

    initFromVertices(shaderProgram, verts, vertex_count);

    // OpenGL has the buffer now, there's no need for us to keep a copy.
    delete [] verts;
}
