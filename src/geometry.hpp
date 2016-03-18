#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

class Geometry
{
public:
    GLuint getVertices() { return geometry_vao; }

protected:
	// Fields related to grid geometry.
	GLuint geometry_vao; // Vertex Array Object
	GLuint geometry_vbo; // Vertex Buffer Object

    void initFromVertices(ShaderProgram& shader_program, float* vertices, int vertex_count);
};
