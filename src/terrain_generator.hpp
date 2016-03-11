#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"

class TerrainGenerator {
public:
    TerrainGenerator();

    void init(std::string dir);
private:
    ShaderProgram terrain_shader;
};
