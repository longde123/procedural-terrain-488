#pragma once

#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>
#include <vector>

#include "cube.hpp"
#include "terrain_generator.hpp"

class Swarm
{
public:
    Swarm();

    void init(std::string dir);
    void initializeAttributes(TerrainGenerator& terrain_generator);
    void draw(glm::mat4 P, glm::mat4 V, glm::mat4 M, glm::vec3 eye_position,
              TerrainGenerator& terrain_generator);

private:
    ShaderProgram initialization_shader;
    ShaderProgram update_shader;

    GLuint positions_buffer;
    GLuint velocities_buffer;
    GLuint colors_buffer;

    GLint block_size_uni;
    GLint period_uni;
    GLint warp_params_uni;

    GLint P_uni;    // Uniform location for Projection matrix.
    GLint V_uni;    // Uniform location for View matrix.
    GLint M_uni;    // Uniform location for Model matrix.

    GLint pos_attrib;
    GLint color_attrib;

    Cube cube;
};
