#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"

class TerrainRenderer {
public:
    TerrainRenderer();

    void init(std::string dir);

    ShaderProgram renderer_shader;

	GLint P_uni;    // Uniform location for Projection matrix.
	GLint V_uni;    // Uniform location for View matrix.
	GLint M_uni;    // Uniform location for Model matrix.
	GLint NormalMatrix_uni;     // Uniform location for Normal matrix.

    GLint posAttrib;
    GLint colorAttrib;
    GLint normalAttrib;
};
