#pragma once

#include <string>
#include <vector>

#include "cs488-framework/ShaderProgram.hpp"
#include "cube.hpp"
#include "lod.hpp"
#include "transform_program.hpp"

// Level-of-detail visualizer.
class LodVisualizer {
public:
    LodVisualizer();

    void init(std::string dir);
    void draw(glm::mat4 P, glm::mat4 V, glm::mat4 W, glm::vec3 current_pos);

private:
    ShaderProgram lod_shader;

    Cube cube;
    Lod lod;

    GLint P_uni;    // Uniform location for Projection matrix.
    GLint V_uni;    // Uniform location for View matrix.
    GLint M_uni;    // Uniform location for Model matrix.
    GLint color_uni;
    GLint fog_uni;
};
