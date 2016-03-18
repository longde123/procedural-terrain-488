#include "grid.hpp"

#include <algorithm>

#include "cs488-framework/GlErrorCheck.hpp"

Grid::Grid(size_t d)
: size(d)
{
}

void Grid::init(ShaderProgram& shaderProgram)
{
	size_t sz = 3 * size * size;

	float *verts = new float[ sz ];
	size_t ct = 0;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            // TODO: For optimization, try swapping x and y and see if that
            // makes any difference
            int idx = x + y * size;
            verts[idx * 3] = x;
            verts[idx * 3 + 1] = 0;
            verts[idx * 3 + 2] = y;
        }
    }

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays(1, &grid_vao);
	glBindVertexArray(grid_vao);

	// Create the cube vertex buffer
	glGenBuffers(1, &grid_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = shaderProgram.getAttribLocation( "position" );
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Reset state to prevent rogue code from messing with *my* stuff!
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}
