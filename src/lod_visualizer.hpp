#pragma once

#include <string>
#include <vector>

#include "cs488-framework/ShaderProgram.hpp"
#include "cube.hpp"
#include "transform_program.hpp"

// Level-of-detail visualizer.
class LodVisualizer {
public:
    LodVisualizer();

    void init(std::string dir);
    void draw(glm::mat4 P, glm::mat4 V, glm::vec3 current_pos);

private:
    void genSubblocks(std::vector<glm::ivec3>& subblocks, int n);

    ShaderProgram lod_shader;

    Cube cube;

	GLint P_uni;    // Uniform location for Projection matrix.
	GLint V_uni;    // Uniform location for View matrix.
	GLint M_uni;    // Uniform location for Model matrix.
    GLint color_uni;

    // Blocks of size 1 within blocks of size 2
    std::vector<glm::ivec3> block_2_subblocks;
    // Blocks of size 2 within blocks of size 4
    std::vector<glm::ivec3> block_4_subblocks;
};
