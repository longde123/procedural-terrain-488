#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"
#include "grid.hpp"
#include "TransformProgram.hpp"

#define BLOCK_DIMENSION 63

class TerrainGenerator {
public:
    TerrainGenerator();

    void init(std::string dir);
    void initBuffer(GLint posAttrib, GLint colorAttrib, GLint normalAttrib);

    void generateTerrainBlock();

    GLuint getVertices() { return out_vao; }

    // A 3D cubic block of terrain.
    GLuint block;

    GLuint feedbackObject;

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
