#include "lod.hpp"

using namespace glm;
using namespace std;

// Range of distance for blocks of size 1 where they fade out.
const int BLOCK_1_FADEOUT_START = 4;
const int BLOCK_1_FADEOUT_END = 5;

// Range of distance for blocks of size 2 where they fade out.
const int BLOCK_2_FADEOUT_START = 8;
const int BLOCK_2_FADEOUT_END = 10;

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

bool Lod::blockIsInView(mat4& P, mat4& V, ivec3 block, int size)
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
        vec4 point = P * V * vec4(cubePoints[i] + vec3(block), 1.0);
        bool inside = true;

        if (point.x < -point.w) {
            inside = false;
        } else {
            all_on_left = false;
        }
        if (point.x > point.w) {
            inside = false;
        } else {
            all_on_right = false;
        }
        if (point.y < -point.w) {
            inside = false;
        } else {
            all_on_bottom = false;
        }
        if (point.y > point.w) {
            inside = false;
        } else {
            all_on_top = false;
        }
        if (point.z < -point.w) {
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

void Lod::generateForPosition(mat4 P, mat4 V, vec3 current_pos)
{
    blocks_of_size_1.clear();
    blocks_of_size_2.clear();
    blocks_of_size_4.clear();

    for (int x = -range; x <= range; x += 1) {
        for (int y = -range; y <= range; y += 1) {
            ivec3 block = ivec3(x, 0, y) + ivec3(current_pos.x, 0, current_pos.z);

            if (!blockIsInView(P, V, block, 1)) {
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

    for (int x = -range; x <= range; x += 2) {
        for (int y = -range; y <= range; y += 2) {
            ivec3 block = ivec3(x, 0, y) + (ivec3(current_pos.x, 0, current_pos.z) / 2) * 2;
            assert(block % 2 == ivec3(0));

            if (!blockIsInView(P, V, block, 2)) {
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
        for (int y = -range; y <= range; y += 4) {
            ivec3 block = ivec3(x, 0, y) + (ivec3(current_pos.x, 0, current_pos.z) / 4) * 4;
            assert(block % 4 == ivec3(0));

            if (!blockIsInView(P, V, block, 4)) {
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
