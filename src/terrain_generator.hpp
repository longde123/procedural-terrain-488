#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"
#include "constants.hpp"
#include "grid.hpp"
#include "transform_program.hpp"

class TerrainGenerator {
public:
    TerrainGenerator();

    void init(std::string dir);
    void initBuffer(GLint pos_attrib, GLint color_attrib, GLint normal_attrib);

    void generateTerrainBlock();

    GLuint getVertices() { return out_vao; }

    // A 3D cubic block of terrain.
    GLuint block;

    GLuint feedback_object;

    float period;
private:
    ShaderProgram density_shader;
    TransformProgram marching_cubes_shader;

    GLint period_uni;

    // Vertices corresponding to grid points used for the geometry shader.
    Grid grid;

    GLuint out_vao;
    GLuint out_vbo;
};
