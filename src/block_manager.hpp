#pragma once

#include <vector>

#include "block.hpp"
#include "lod.hpp"

#include "terrain_generator_slow.hpp"
#include "terrain_generator_medium.hpp"
#include "terrain_renderer.hpp"

#include "water.hpp"

class BlockManager {
public:
    BlockManager();

    void init(std::string dir);
    void regenerateBlocks();
    void renderBlocks(glm::mat4 P, glm::mat4 V, glm::mat4 W, glm::vec3 eye_position);

    std::vector<Block> blocks;

    bool triplanar_colors;
    bool use_ambient;
    bool use_normal_map;
    bool debug_flag;
    bool use_water;
    float water_height;
    float light_x;

    TerrainRenderer terrain_renderer;
    //TerrainGeneratorSlow terrain_generator;
    TerrainGeneratorMedium terrain_generator;
private:
    Lod lod;
    Water water;
};
