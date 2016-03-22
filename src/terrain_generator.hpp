#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"
#include "constants.hpp"
#include "grid.hpp"
#include "transform_program.hpp"

class TerrainGenerator {
public:
    TerrainGenerator();
    virtual ~TerrainGenerator() {}

    void init(std::string dir);
    void initBuffer(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib);

    virtual void generateTerrainBlock() = 0;

    GLuint getVertices() { return out_vao; }

    // A 3D cubic block of terrain.
    GLuint block;

    GLuint feedback_object;

    float period;
    bool use_short_range_ambient_occlusion;
    bool use_long_range_ambient_occlusion;

protected:
    void generateDensity();

    ShaderProgram density_shader;

    GLint period_uni;

    GLuint out_vao;
    GLuint out_vbo;
};
