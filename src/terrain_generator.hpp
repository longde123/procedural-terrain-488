#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"

#define BLOCK_DIMENSION 32

class TerrainGenerator {
public:
    TerrainGenerator();

    void init(std::string dir);

    void generateTerrainBlock();

    // A 3D cubic block of terrain.
    GLuint block;
private:
    ShaderProgram terrain_shader;
};
