#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"
#include "block.hpp"
#include "constants.hpp"
#include "grid.hpp"
#include "transform_program.hpp"

class TerrainGenerator {
public:
    TerrainGenerator();
    virtual ~TerrainGenerator() {}

    void init(std::string dir);

    virtual void generateTerrainBlock(Block& block) = 0;

    float period;
    bool use_short_range_ambient_occlusion;
    bool use_long_range_ambient_occlusion;

protected:
    void generateDensity(Block block);

private:
    ShaderProgram density_shader;

    // A 3D cubic block of terrain.
    GLuint block_texture;

    GLint period_uni;
    GLint block_index_uni;
};
