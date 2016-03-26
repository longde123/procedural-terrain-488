#pragma once

#include <queue>
#include <memory>
#include <unordered_map>
#include <vector>

#include "block.hpp"
#include "lod.hpp"

#include "terrain_generator_slow.hpp"
#include "terrain_generator_medium.hpp"
#include "terrain_renderer.hpp"

#include "water.hpp"

struct KeyHash {
    std::size_t operator()(const glm::ivec4& v) const
    {
        long x = v.x;
        long y = v.y;
        long z = v.z;
        long w = v.w;
        return std::hash<long>()(x + (y << 8) + (z << 16) + (w << 24));
    }
};

struct KeyEqual {
    bool operator()(const glm::ivec4& lhs, const glm::ivec4& rhs) const
    {
        return lhs == rhs;
    }
};

class BlockManager {
public:
    BlockManager();

    void init(std::string dir);
    void update(glm::vec3 eye_position, bool generate_blocks);
    void regenerateAllBlocks();
    void renderBlocks(glm::mat4 P, glm::mat4 V, glm::mat4 W, glm::vec3 eye_position);

    std::unordered_map<glm::ivec4, std::shared_ptr<Block>, KeyHash, KeyEqual> blocks;

    bool triplanar_colors;
    bool use_ambient;
    bool use_normal_map;
    bool debug_flag;
    bool use_water;
    bool small_blocks;
    bool medium_blocks;
    bool large_blocks;
    float water_height;
    float light_x;

    TerrainRenderer terrain_renderer;
    //TerrainGeneratorSlow terrain_generator;
    TerrainGeneratorMedium terrain_generator;
private:
    void renderBlock(glm::mat4 P, glm::mat4 V, glm::mat4 W, Block& block, float fadeAlpha);
    void newBlock(glm::ivec3 index, int size);

    Lod lod;
    Water water;

    // List of blocks to generate.
    std::queue<std::shared_ptr<Block>> block_queue;
};
