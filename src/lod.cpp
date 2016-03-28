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
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                subblocks.push_back(ivec3(i, j, k));
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

void Lod::generateForPosition(mat4 P, mat4 V, mat4 W, vec3 current_pos)
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
                    blocks_of_size_1.push_back(make_pair(block, 1.0f - clamp(ratio, 0.0f, 1.0f)));
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
                blocks_of_size_2.push_back(make_pair(block, 1.0f - clamp(ratio, 0.0f, 1.0f)));
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
            for (ivec3& subblocks : block_4_subblocks) {
                if (length(vec3(block + subblocks) - current_pos) > BLOCK_2_FADEOUT_START) {
                    fully_visible_subblocks = false;
                    break;
                }
            }

            if (!fully_visible_subblocks) {
                blocks_of_size_4.push_back(make_pair(block, 1.0f));
            }
        }
    }
}
