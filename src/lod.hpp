#pragma once

#include <vector>

#include <glm/glm.hpp>

// Calculate level of detail required for blocks
class Lod {
public:
    Lod(int range);

    void generateForPosition(glm::vec3 current_pos);

    // Contains xyz coordinates and w is the alpha of the block.
    std::vector<glm::ivec4> blocks_of_size_1;
    std::vector<glm::ivec4> blocks_of_size_2;
    std::vector<glm::ivec4> blocks_of_size_4;
private:
    void genSubblocks(std::vector<glm::ivec3>& subblocks, int n);

    int range;

    // Blocks of size 1 within blocks of size 2
    std::vector<glm::ivec3> block_2_subblocks;
    // Blocks of size 2 within blocks of size 4
    std::vector<glm::ivec3> block_4_subblocks;
};
