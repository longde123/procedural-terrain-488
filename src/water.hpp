#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"
#include "constants.hpp"
#include "plane.hpp"

class Water
{
public:
    Water();

    void init(std::string dir);
    void draw(glm::mat4 P, glm::mat4 V, glm::mat4 M);

private:
    ShaderProgram water_shader;

    Plane water_plane;

	GLint P_uni;    // Uniform location for Projection matrix.
	GLint V_uni;    // Uniform location for View matrix.
	GLint M_uni;    // Uniform location for Model matrix.
};
