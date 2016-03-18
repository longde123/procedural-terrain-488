#include "grid.hpp"

#include <algorithm>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;

Grid::Grid(size_t d)
: size(d)
{
}

void Grid::init(ShaderProgram& shaderProgram, mat4 transform)
{
	size_t vertex_count = 3 * size * size;

	float *verts = new float[vertex_count];
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            vec3 point = vec3(transform * vec4(x, 0, y, 1));

            int idx = x + y * size;
            verts[idx * 3] = point.x;
            verts[idx * 3 + 1] = point.y;
            verts[idx * 3 + 2] = point.z;
        }
    }

    initFromVertices(shaderProgram, verts, vertex_count);

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;
}
