#pragma once

#include "terrain_generator.hpp"

class TerrainGeneratorSlow : public TerrainGenerator {
public:
    TerrainGeneratorSlow();
    virtual ~TerrainGeneratorSlow() {}

    void init(std::string dir);

    virtual void generateTerrainBlock(Block& block);

private:
    TransformProgram marching_cubes_shader;

    GLint block_size_uni;
    GLint block_padding_uni_marching;
    GLint period_uni_marching;
    GLint octaves_uni_marching;
    GLint octaves_decay_uni_marching;
    GLint warp_params_uni_marching;
    GLint short_range_ambient_uni;
    GLint long_range_ambient_uni;

    // Vertices corresponding to grid points used for the geometry shader.
    Grid grid;
};
