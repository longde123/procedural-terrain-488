#include "lod.hpp"

using namespace glm;
using namespace std;

// Range of distance for blocks of size 1 where they fade out.
const int BLOCK_1_FADEOUT_START = 5;
const int BLOCK_1_FADEOUT_END = 6;

// Range of distance for blocks of size 2 where they fade out.
const int BLOCK_2_FADEOUT_START = 12;
const int BLOCK_2_FADEOUT_END = 14;

Lod::Lod(int range)
: range(range)
{

    genSubblocks(block_2_subblocks, 2);
    genSubblocks(block_4_subblocks, 4);
}

void Lod::genSubblocks(vector<ivec3>& subblocks, int n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                subblocks.push_back(ivec3(i, j, k));
            }
        }
    }
}

void Lod::generateForPosition(vec3 current_pos)
{
    blocks_of_size_1.clear();
    blocks_of_size_2.clear();
    blocks_of_size_4.clear();

    for (int x = -range; x <= range; x += 1) {
        for (int y = -range; y <= range; y += 1) {
            ivec3 block = ivec3(x, 0, y) + ivec3(current_pos.x, 0, current_pos.z);

            float distance = length(vec3(block) - current_pos);

            if (distance < BLOCK_1_FADEOUT_END) {
                float ratio = (distance - BLOCK_1_FADEOUT_START) /
                              (BLOCK_1_FADEOUT_END - BLOCK_1_FADEOUT_START);
                blocks_of_size_1.push_back(vec4(block, 1.0f - clamp(ratio, 0.0f, 1.0f)));
            }
        }
    }

    for (int x = -range; x <= range; x += 2) {
        for (int y = -range; y <= range; y += 2) {
            ivec3 block = ivec3(x, 0, y) + (ivec3(current_pos.x, 0, current_pos.z) / 2) * 2;
            assert(block % 2 == ivec3(0));

            bool fully_visible_subblocks = true;
            for (ivec3& subblocks : block_2_subblocks) {
                if (length(vec3(block + subblocks) - current_pos) > BLOCK_1_FADEOUT_START) {
                    fully_visible_subblocks = false;
                    break;
                }
            }

            float distance = length(vec3(block) - current_pos);

            if (!fully_visible_subblocks && distance < BLOCK_2_FADEOUT_END) {
                float ratio = (distance - BLOCK_2_FADEOUT_START) /
                              (BLOCK_2_FADEOUT_END - BLOCK_2_FADEOUT_START);
                blocks_of_size_2.push_back(vec4(block, 1.0f - clamp(ratio, 0.0f, 1.0f)));
            }
        }
    }

    for (int x = -range; x <= range; x += 4) {
        for (int y = -range; y <= range; y += 4) {
            ivec3 block = ivec3(x, 0, y) + (ivec3(current_pos.x, 0, current_pos.z) / 4) * 4;
            assert(block % 4 == ivec3(0));

            bool fully_visible_subblocks = true;
            for (ivec3& subblocks : block_4_subblocks) {
                if (length(vec3(block + subblocks) - current_pos) > BLOCK_2_FADEOUT_START) {
                    fully_visible_subblocks = false;
                    break;
                }
            }

            if (!fully_visible_subblocks) {
                blocks_of_size_4.push_back(vec4(block, 1.0f));
            }
        }
    }
}
