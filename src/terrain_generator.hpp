#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"

#define BLOCK_DIMENSION 256

class TerrainGenerator {
public:
    TerrainGenerator();

    void init(std::string dir);

    void generateTerrainBlock();
private:
    ShaderProgram terrain_shader;

    // A 3D cubic block of terrain.
    GLuint block;
};
