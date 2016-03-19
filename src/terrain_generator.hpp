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
    void initBuffer(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib);

    void generateTerrainBlock();

    GLuint getVertices() { return out_vao; }

    // A 3D cubic block of terrain.
    GLuint block;

    GLuint feedback_object;

    float period;
    bool use_short_range_ambient_occlusion;
    bool use_long_range_ambient_occlusion;
private:
    ShaderProgram density_shader;
    TransformProgram marching_cubes_shader;

    GLint period_uni;
    GLint period_uni_marching;
    GLint short_range_ambient_uni;
    GLint long_range_ambient_uni;

    // Vertices corresponding to grid points used for the geometry shader.
    Grid grid;

    GLuint out_vao;
    GLuint out_vbo;
};
