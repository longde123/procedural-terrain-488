#pragma once

#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

class Grid
{
public:
	Grid( size_t dim );

    void init(ShaderProgram& shaderProgram);

    GLuint getVertices() { return grid_vao; }

private:
	// Fields related to grid geometry.
	GLuint grid_vao; // Vertex Array Object
	GLuint grid_vbo; // Vertex Buffer Object

    size_t size;
};
