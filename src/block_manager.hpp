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
    std::size_t operator()(const glm::ivec2& v) const {
        long x = v.x;
        long y = v.y;
        return std::hash<long>()(x + (y << 8));
    }
    std::size_t operator()(const glm::ivec4& v) const {
        long x = v.x;
        long y = v.y;
        long z = v.z;
        long w = v.w;
        return std::hash<long>()(x + (y << 8) + (z << 16) + (w << 24));
    }
};

struct KeyEqual {
    bool operator()(const glm::ivec2& lhs, const glm::ivec2& rhs) const
    {
        return lhs == rhs;
    }
    bool operator()(const glm::ivec4& lhs, const glm::ivec4& rhs) const
    {
        return lhs == rhs;
    }
};

enum TerrainGeneratorSelection {
    Slow = 0,
    Medium = 1,
};

enum BlockDisplayType {
    OneBlock = 0,
    EightBlocks = 1,
    All = 2,
};

typedef std::unordered_map<glm::ivec2, float, KeyHash, KeyEqual> ivec2float_map;

class BlockManager {
public:
    BlockManager();

    void init(std::string dir);
    void update(float time_elapsed, glm::mat4 P, glm::mat4 V, glm::mat4 W,
                glm::vec3 eye_position, bool generate_blocks);
    void regenerateAllBlocks(bool alpha_blend = true);
    void renderBlocks(glm::mat4 P, glm::mat4 V, glm::mat4 W, glm::vec3 eye_position);

    void profileBlockGeneration();

    int blocksInQueue() { return blocks_in_queue; }
    int blocksInView() { return blocks_in_view; }

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

    BlockDisplayType block_display_type;
    TerrainGeneratorSelection generator_selection;

    TerrainRenderer terrain_renderer;
    TerrainGenerator* terrain_generator;
private:
    void renderBlock(glm::mat4 P, glm::mat4 V, glm::mat4 W, Block& block, float fadeAlpha);
    void processBlockOfSize(glm::mat4 P, glm::mat4 V, glm::mat4 W,
                            ivec2float_map& water_squares,
                            glm::ivec3 position, int size, float alpha);
    std::shared_ptr<Block> newBlock(glm::ivec3 index, int size);
    void generateBestBlock();

    // Keep track of this for debugging.
    int blocks_in_view;
    int blocks_in_queue;

    Lod lod;
    Water water;
    TerrainGeneratorSlow terrain_generator_slow;
    TerrainGeneratorMedium terrain_generator_medium;

    // List of blocks to generate.
    std::queue<std::shared_ptr<Block>> block_queue;
};
