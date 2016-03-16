#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"

#define BLOCK_DIMENSION 31

class TerrainGenerator {
public:
    TerrainGenerator();

    void init(std::string dir);

    void generateTerrainBlock();

    // A 3D cubic block of terrain.
    GLuint block;

    float period;
private:
    ShaderProgram terrain_shader;

    GLint period_uni;
};
