#include "block_manager.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <unordered_set>

#include "cs488-framework/GlErrorCheck.hpp"

#include "timer.hpp"

using namespace glm;
using namespace std;

static ivec4_set eight_blocks;

BlockManager::BlockManager()
: lod(VIEW_RANGE)
{
    triplanar_colors = false;
    use_ambient = true;
    use_normal_map = true;
    debug_flag = false;
    show_ambient = false;
    use_water = true;
    use_stencil = true;
    water_height = -0.3f;
    small_blocks = true;
    medium_blocks = true;
    large_blocks = true;
    blocks_per_frame = 2;

    light_x = 0.0f;
    light_ambient = vec3(0.2);
    light_diffuse = vec3(0.6);
    light_specular = vec3(0.2);

    reused_block_count = 0;

    terrain_generator = &terrain_generator_medium;
    block_display_type = All;
    generator_selection = Medium;

    eight_blocks.insert(ivec4(0, 0, 0, 1));
    eight_blocks.insert(ivec4(1, 0, 0, 1));
    eight_blocks.insert(ivec4(0, 1, 0, 1));
    eight_blocks.insert(ivec4(1, 1, 0, 1));
    eight_blocks.insert(ivec4(0, 0, 1, 1));
    eight_blocks.insert(ivec4(1, 0, 1, 1));
    eight_blocks.insert(ivec4(0, 1, 1, 1));
    eight_blocks.insert(ivec4(1, 1, 1, 1));
}

void BlockManager::init(string dir)
{
    terrain_renderer.init(dir);
    terrain_generator_slow.init(dir);
    terrain_generator_medium.init(dir);
    water.init(dir);
}

void BlockManager::profileBlockGeneration()
{
    // Make sure OpenGL has executed everything, they don't interfere
    // with our profiling.
    glFinish();

    Timer timer;
    timer.start();

    auto block = shared_ptr<Block>(new Block(ivec3(0), 1));
    block->init(terrain_renderer.pos_attrib, terrain_renderer.normal_attrib,
                terrain_renderer.ambient_occlusion_attrib);

    for (int i = 0; i < 100; i++) {
        terrain_generator->generateTerrainBlock(*block);
    }

    // Make sure OpenGL has executed everything, so that they are measured
    // by the profiling.
    glFinish();

    timer.stop();

    printf("Generating 100 blocks took %f seconds\n", timer.elapsedSeconds());
}

int BlockManager::allocatedBlocks()
{
    return free_blocks.size() + blocks.size();
}

void BlockManager::regenerateAllBlocks(bool alpha_blend)
{
    for (auto& kv : blocks) {
        auto& block = kv.second;
        block->resetBlock(alpha_blend);
        free_blocks.push(block);
    }

    blocks.clear();

    // We might want to regenerate this block continuously when we show
    // only few blocks, so generate themn ow.
    if (block_display_type == OneBlock) {
        if (blocks.count(ivec4(0, 0, 0, 1))) {
            auto block = blocks[ivec4(0, 0, 0, 1)];
            terrain_generator->generateTerrainBlock(*block);
            block->finish();
        }
    } else if (block_display_type == EightBlocks) {
        for (ivec4 index : eight_blocks) {
            if (blocks.count(index) == 0) {
                newBlock(ivec3(index), index.w);
            }
            auto block = blocks[index];
            terrain_generator->generateTerrainBlock(*block);
            block->finish();
        }
    }
}

void BlockManager::generateBestBlock()
{
    // Generate fully visible blocks first. This lets us have at least something,
    // even if it's lower detail. If a block is not fully visible, this means it's
    // transitioning, so there is at least a fully visible block under it.
    //
    // We'd like to generate them in order of distance to the camera.
    // For now I'm being lazy and just generating small blocks first,
    // then medium ones, then large ones, as a proxy.
    for (auto& block : lod.blocks_of_size_4) {
        if (blocks.count(ivec4(block.first, 4)) == 0 && block.second == 1.0) {
            auto new_block = newBlock(block.first, 4);
            terrain_generator->generateTerrainBlock(*new_block);
            new_block->finish();
            return;
        }
    }
    for (auto& block : lod.blocks_of_size_2) {
        if (blocks.count(ivec4(block.first, 2)) == 0 && block.second == 1.0) {
            auto new_block = newBlock(block.first, 2);
            terrain_generator->generateTerrainBlock(*new_block);
            new_block->finish();
            return;
        }
    }
    for (auto& block : lod.blocks_of_size_1) {
        if (blocks.count(ivec4(block.first, 1)) == 0 && block.second == 1.0) {
            auto new_block = newBlock(block.first, 1);
            terrain_generator->generateTerrainBlock(*new_block);
            new_block->finish();
            return;
        }
    }

    // Generate non-fully visible blocks.
    for (auto& block : lod.blocks_of_size_4) {
        if (blocks.count(ivec4(block.first, 4)) == 0) {
            auto new_block = newBlock(block.first, 4);
            terrain_generator->generateTerrainBlock(*new_block);
            new_block->finish();
            return;
        }
    }
    for (auto& block : lod.blocks_of_size_2) {
        if (blocks.count(ivec4(block.first, 2)) == 0) {
            auto new_block = newBlock(block.first, 2);
            terrain_generator->generateTerrainBlock(*new_block);
            new_block->finish();
            return;
        }
    }
    for (auto& block : lod.blocks_of_size_1) {
        if (blocks.count(ivec4(block.first, 1)) == 0) {
            auto new_block = newBlock(block.first, 1);
            terrain_generator->generateTerrainBlock(*new_block);
            new_block->finish();
            return;
        }
    }
}

void BlockManager::update(float time_elapsed, mat4 P, mat4 V, mat4 W, vec3 eye_position, bool generate_blocks)
{
    switch (generator_selection) {
        case Slow:
            terrain_generator = &terrain_generator_slow;
            break;
        case Medium:
            terrain_generator = &terrain_generator_medium;
            break;
    }

    ivec4_map<float> existing_blocks_alpha;
    for (auto& kv : blocks) {
        existing_blocks_alpha[kv.first] = kv.second->getAlpha();
    }
    lod.generateForPosition(P, V, W, eye_position, &existing_blocks_alpha);

    blocks_in_view = lod.blocks_of_size_1.size() + lod.blocks_of_size_2.size() + lod.blocks_of_size_4.size();

    // Count blocks that we don't already have.
    blocks_in_queue = 0;

    for (auto& block : lod.blocks_of_size_4) {
        if (blocks.count(ivec4(block.first, 4)) == 0) {
            blocks_in_queue++;
        }
    }
    for (auto& block : lod.blocks_of_size_2) {
        if (blocks.count(ivec4(block.first, 2)) == 0) {
            blocks_in_queue++;
        }
    }
    for (auto& block : lod.blocks_of_size_1) {
        if (blocks.count(ivec4(block.first, 1)) == 0) {
            blocks_in_queue++;
        }
    }

    for (int i = 0; i < blocks_per_frame; i++) {
        generateBestBlock();
    }

    vector<ivec4> to_be_removed;
    for (auto& kv : blocks) {
        auto& block = kv.second;

        float distance = length(eye_position - vec3(block->index));
        if (distance > VIEW_RANGE * 1.1) {
            // Block is no longer visible, remove, but only if size 1 or 2.
            if (block->size < 4) {
                to_be_removed.push_back(kv.first);
                block->resetBlock();
                free_blocks.push(block);
            } else if (distance > VIEW_RANGE * 2) {
                // Should still remove large blocks at some point.
                to_be_removed.push_back(kv.first);
                block->resetBlock();
                free_blocks.push(block);
            }
        }

        if (block->isReady()) {
            block->update(time_elapsed);
        }
    }

    for (auto& index : to_be_removed) {
        blocks.erase(index);
    }
}

shared_ptr<Block> BlockManager::newBlock(ivec3 index, int size)
{
    shared_ptr<Block> block;
    if (free_blocks.empty()) {
        block = shared_ptr<Block>(new Block(index, size));
        block->init(terrain_renderer.pos_attrib, terrain_renderer.normal_attrib,
                    terrain_renderer.ambient_occlusion_attrib);
    } else {
        reused_block_count++;
        block = free_blocks.front();
        free_blocks.pop();

        block->index = index;
        block->size = size;
    }
    blocks[ivec4(index, size)] = block;
    return block;
}

void BlockManager::renderBlock(mat4 P, mat4 V, mat4 W, Block& block, float fadeAlpha)
{
    assert(block.isReady());

    mat4 block_transform = translate(vec3(block.index)) * W * scale(vec3(block.size));
    mat3 normalMatrix = mat3(transpose(inverse(block_transform)));
    glUniformMatrix4fv(terrain_renderer.M_uni, 1, GL_FALSE, value_ptr(block_transform));
    glUniformMatrix3fv(terrain_renderer.NormalMatrix_uni, 1, GL_FALSE, value_ptr(normalMatrix));

    glUniform1f(terrain_renderer.alpha_uni, std::min(block.getAlpha(), fadeAlpha));

    glBindVertexArray(block.out_vao);

    if (use_water) {
        glUniform1i(terrain_renderer.water_clip_uni, true);
        glUniform1i(terrain_renderer.water_reflection_clip_uni, false);
        glDrawTransformFeedback(GL_TRIANGLES, block.feedback_object);

        mat4 W_reflect = glm::translate(vec3(0, water_height, 0)) *
                         glm::scale(vec3(1.0f, -1.0f, 1.0f)) *
                         glm::translate(vec3(0, -water_height, 0)) *
                         translate(vec3(block.index)) *
                         W * scale(vec3(block.size));

        if (use_stencil && block_display_type != All) {
            glEnable(GL_STENCIL_TEST);
        }

        // Draw the reflection.
        glUniform1i(terrain_renderer.water_clip_uni, false);
        glUniform1i(terrain_renderer.water_reflection_clip_uni, true);
        glUniformMatrix4fv( terrain_renderer.M_uni, 1, GL_FALSE, value_ptr(W_reflect));
        glDrawTransformFeedback(GL_TRIANGLES, block.feedback_object);

        if (use_stencil && block_display_type != All) {
            glDisable(GL_STENCIL_TEST);
        }
    } else {
        glUniform1i(terrain_renderer.water_clip_uni, false);
        glUniform1i(terrain_renderer.water_reflection_clip_uni, false);
        glDrawTransformFeedback(GL_TRIANGLES, block.feedback_object);
    }

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

void BlockManager::processBlockOfSize(mat4 P, mat4 V, mat4 W,
                                      ivec2_map<float>& water_squares,
                                      ivec3 position, int size, float alpha)
{
    ivec4 index = vec4(position, size);
    if (blocks.count(index) > 0 && blocks[index]->isReady()) {
        // Don't draw blocks under water.
        if (!use_water || (W * vec4(position, 1.0)).y + size >= water_height) {
            renderBlock(P, V, W, *blocks[index], alpha);
        }

        // Indicate grid units that need water corresponding to this block.
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                ivec2 water_index = ivec2(index.x + x, index.z + y);
                if (water_squares.count(water_index) > 0) {
                    water_squares[water_index] = std::max(alpha, water_squares[water_index]);
                } else {
                    water_squares[water_index] = alpha;
                }
            }
        }
    }
}

void BlockManager::renderStencil(mat4 P, mat4 V, mat4 W) {
    // Refer to https://open.gl/depthstencils
    glEnable(GL_STENCIL_TEST);

    // Draw water in stencil mode.
    glStencilFunc(GL_ALWAYS, 1, 0xFF);  // Set any stencil to 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);                // Write to stencil buffer
    glDepthMask(GL_FALSE);              // Don't write to depth buffer
    glClear(GL_STENCIL_BUFFER_BIT);     // Clear stencil buffer (0 by default)

    water.start();

    mat4 stencil_transform;
    if (block_display_type == OneBlock) {
        stencil_transform = W;
    } else {
        stencil_transform = W * scale(vec3(2));
    }
    water.draw(P, V, glm::translate(vec3(0, water_height + 0.5f, 0)) * stencil_transform,
               vec3(0), 1.0);

    water.end();

    // Setup stencil to draw the reflection.
    glStencilFunc(GL_EQUAL, 1, 0xFF);   // Pass test if stencil value is 1
    glStencilMask(0x00);                // Don't write anything to stencil buffer
    glDepthMask(GL_TRUE);               // Write to depth buffer

    // Turn it and just leave the stencil there, the block renderer will turn it
    // back on and off.
    glDisable(GL_STENCIL_TEST);
}

void BlockManager::renderBlocks(mat4 P, mat4 V, mat4 W, vec3 eye_position)
{
    // We need to make sure not to draw water multiple times on the same grid
    // cell, because the overlapping will cause visual artifacts.
    // Keep track of the highest alpha at that cell.
    ivec2_map<float> needed_water_squares;

    if (use_stencil && block_display_type != All) {
        // We don't need stencils when all blocks are displayed since we
        // won't see the bits of the reflected geometry that are beyond
        // the water plane anyway.
        renderStencil(P, V, W);
    }
    glClear(GL_STENCIL_BUFFER_BIT);     // Clear stencil buffer (0 by default)

    terrain_renderer.renderer_shader.enable();
        glUniformMatrix4fv(terrain_renderer.P_uni, 1, GL_FALSE, value_ptr(P));
        glUniformMatrix4fv(terrain_renderer.V_uni, 1, GL_FALSE, value_ptr(V));

        glUniform1i(terrain_renderer.triplanar_colors_uni, triplanar_colors);
        glUniform1i(terrain_renderer.show_ambient_uni, show_ambient);
        glUniform1i(terrain_renderer.use_ambient_uni, use_ambient);
        glUniform1i(terrain_renderer.use_normal_map_uni, use_normal_map);
        glUniform1i(terrain_renderer.debug_flag_uni, debug_flag);

        glUniform1f(terrain_renderer.clip_height_uni, water_height);

        glUniform3f(terrain_renderer.eye_position_uni, eye_position.x, eye_position.y, eye_position.z);

        glUniform3f(terrain_renderer.light_position_uni, 30.0f, 50.0f, light_x);
        glUniform3f(terrain_renderer.light_ambient_uni, light_ambient.r, light_ambient.g, light_ambient.b);
        glUniform3f(terrain_renderer.light_diffuse_uni, light_diffuse.r, light_diffuse.g, light_diffuse.b);
        glUniform3f(terrain_renderer.light_specular_uni, light_specular.r, light_specular.g, light_specular.b);

        glUniform3f(terrain_renderer.fog_uni, FOG_MULTIPLIER, VIEW_RANGE, FOG_BIAS);

        terrain_renderer.prepareRender();

        glEnable(GL_CLIP_DISTANCE0);

        if (large_blocks) {
            for (auto& block : lod.blocks_of_size_4) {
                if (block_display_type != All) {
                    continue;
                }
                processBlockOfSize(P, V, W, needed_water_squares, block.first, 4, block.second);
            }
        }

        if (medium_blocks) {
            for (auto& block : lod.blocks_of_size_2) {
                if (block_display_type != All) {
                    continue;
                }
                processBlockOfSize(P, V, W, needed_water_squares, block.first, 2, block.second);
            }
        }

        if (small_blocks) {
            for (auto& block : lod.blocks_of_size_1) {
                if (block_display_type == OneBlock && block.first != ivec3(0, 0, 0)) {
                    continue;
                }
                if (block_display_type == EightBlocks && eight_blocks.count(ivec4(block.first, 1)) == 0) {
                    continue;
                }
                processBlockOfSize(P, V, W, needed_water_squares, block.first, 1, block.second);
            }
        }

        glDisable(GL_CLIP_DISTANCE0);

        // Draw the cubes
        // Highlight the active square.
    terrain_renderer.renderer_shader.disable();
    if (use_water) {
        water.start();

        for (auto& kv : needed_water_squares) {
            vec3 position = vec3(kv.first.x, 0.0, kv.first.y);
            float alpha = kv.second;
            mat4 block_transform = translate(position) * W;
            water.draw(P, V, glm::translate(vec3(0, water_height + 0.5f, 0)) * block_transform, eye_position, alpha);
        }

        water.end();
    }

    CHECK_GL_ERRORS;
}
