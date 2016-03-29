#include "lod.hpp"
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;
using namespace std;

// Range of distance for blocks of size 1 where they fade out.
const int BLOCK_1_FADEOUT_START = 6;
const int BLOCK_1_FADEOUT_END = 8;

// Range of distance for blocks of size 2 where they fade out.
const int BLOCK_2_FADEOUT_START = 10;
const int BLOCK_2_FADEOUT_END = 12;

Lod::Lod(int range)
: range(range)
{

    genSubblocks(block_2_subblocks, 2);
    genSubblocks(block_4_subblocks, 4);
}

void Lod::genSubblocks(vector<ivec3>& subblocks, int n)
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                subblocks.push_back(ivec3(i, j, k) * n / 2);
            }
        }
    }
}

bool Lod::blockIsInView(mat4& P, mat4& V, mat4& W, ivec3 block, int size)
{
    // This is not a perfect test, but it should do for now.
    vector<vec3> cubePoints = {
        vec3(0, 0, 0),
        vec3(0, size, 0),
        vec3(size, 0, 0),
        vec3(size, size, 0),
        vec3(0, 0, size),
        vec3(0, size, size),
        vec3(size, 0, size),
        vec3(size, size, size),
    };

    bool all_on_left = true;
    bool all_on_right = true;
    bool all_on_top = true;
    bool all_on_bottom = true;
    bool all_behind = true;

    for (int i = 0; i < cubePoints.size(); i++) {
        vec4 point = P * V * translate(vec3(block)) * W  *vec4(cubePoints[i], 1.0);
        bool inside = true;

        float w = point.w;

        if (point.x < -w) {
            inside = false;
        } else {
            all_on_left = false;
        }
        if (point.x > w) {
            inside = false;
        } else {
            all_on_right = false;
        }
        if (point.y < -w) {
            inside = false;
        } else {
            all_on_bottom = false;
        }
        if (point.y > w) {
            inside = false;
        } else {
            all_on_top = false;
        }
        if (point.z < -w) {
            inside = false;
        } else {
            all_behind = false;
        }

        if (inside) {
            return true;
        }
    }

    if (all_on_left || all_on_right || all_on_top || all_on_bottom || all_behind) {
        return false;
    } else {
        return true;
    }
}

void Lod::generateForPosition(mat4 P, mat4 V, mat4 W, vec3 current_pos, ivec4_map<float>* existing_blocks_alpha)
{
    blocks_of_size_1.clear();
    blocks_of_size_2.clear();
    blocks_of_size_4.clear();

    for (int x = -range; x <= range; x += 1) {
        for (int y = 0; y < 2; y += 1) {
            for (int z = -range; z <= range; z += 1) {
                ivec3 block = ivec3(x, y, z) + ivec3(current_pos.x, 0, current_pos.z);

                if (!blockIsInView(P, V, W, block, 1)) {
                    continue;
                }

                float distance = length(vec3(block) - current_pos);

                if (distance < BLOCK_1_FADEOUT_END) {
                    float ratio = (distance - BLOCK_1_FADEOUT_START) /
                                  (BLOCK_1_FADEOUT_END - BLOCK_1_FADEOUT_START);
                    float alpha = 1.0f - clamp(ratio, 0.0f, 1.0f);
                    if (alpha > 0.0) {
                        blocks_of_size_1.push_back(make_pair(block, alpha));
                    }
                }
            }
        }
    }

    for (int x = -range; x <= range; x += 2) {
        for (int z = -range; z <= range; z += 2) {
            ivec3 block = ivec3(x, 0, z) + (ivec3(current_pos.x, 0, current_pos.z) / 2) * 2;
            assert(block % 2 == ivec3(0));

            if (!blockIsInView(P, V, W, block, 2)) {
                continue;
            }

            bool fully_visible_subblocks = true;
            for (ivec3& subblock : block_2_subblocks) {
                ivec3 subblock_index = block + subblock;

                float dist_to_subblock = length(vec3(block + subblock) - current_pos);
                if (dist_to_subblock > BLOCK_1_FADEOUT_START && dist_to_subblock < BLOCK_1_FADEOUT_END) {
                    fully_visible_subblocks = false;
                    break;
                }
                if (existing_blocks_alpha) {
                    if (existing_blocks_alpha->count(ivec4(subblock_index, 1)) > 0) {
                        float alpha = existing_blocks_alpha->at(ivec4(subblock_index, 1));
                        if (alpha < 1.0) {
                            // Subblock still in the process of fading in.
                            fully_visible_subblocks = false;
                            break;
                        }
                    } else {
                        // Subblock needed, but has not been created yet.
                        fully_visible_subblocks = false;
                        break;
                    }
                }
            }

            float distance = length(vec3(block) - current_pos);

            if (!fully_visible_subblocks && distance < BLOCK_2_FADEOUT_END) {
                float ratio = (distance - BLOCK_2_FADEOUT_START) /
                              (BLOCK_2_FADEOUT_END - BLOCK_2_FADEOUT_START);
                float alpha = 1.0f - clamp(ratio, 0.0f, 1.0f);
                if (alpha > 0.0) {
                    blocks_of_size_2.push_back(make_pair(block, alpha));
                }
            }
        }
    }

    if (range % 4 != 0) {
        return;
    }

    for (int x = -range; x <= range; x += 4) {
        for (int z = -range; z <= range; z += 4) {
            ivec3 block = ivec3(x, 0, z) + (ivec3(current_pos.x, 0, current_pos.z) / 4) * 4;
            assert(block % 4 == ivec3(0));

            if (!blockIsInView(P, V, W, block, 4)) {
                continue;
            }

            bool fully_visible_subblocks = true;
            for (ivec3& subblock : block_4_subblocks) {
                ivec3 subblock_index = block + subblock;

                // Subblock too far to be displayed.
                if (length(vec3(subblock_index) - current_pos) > BLOCK_2_FADEOUT_START) {
                    fully_visible_subblocks = false;
                    break;
                }
                if (existing_blocks_alpha) {
                    if (existing_blocks_alpha->count(ivec4(subblock_index, 2)) > 0) {
                        float alpha = existing_blocks_alpha->at(ivec4(subblock_index, 2));
                        if (alpha < 1.0) {
                            // Subblock still in the process of fading in.
                            fully_visible_subblocks = false;
                            break;
                        }
                    } else {
                        // Subblock needed, but has not been created yet.
                        fully_visible_subblocks = false;
                        break;
                    }
                }
            }

            if (!fully_visible_subblocks) {
                blocks_of_size_4.push_back(make_pair(block, 1.0f));
            }
        }
    }
}
