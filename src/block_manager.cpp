#include "block_manager.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

#include "timer.hpp"

using namespace glm;
using namespace std;

BlockManager::BlockManager()
: lod(16)
{
    triplanar_colors = false;
    use_ambient = true;
    use_normal_map = true;
    debug_flag = false;
    use_water = true;
    water_height = 0.0f;
    one_block_only = false;
    small_blocks = true;
    medium_blocks = true;
    large_blocks = true;
    light_x = 0.0f;

    terrain_generator = &terrain_generator_medium;
    generator_selection = Medium;
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

void BlockManager::regenerateAllBlocks()
{
    // Get rid of existing stuff, we're interrupting.
    while (!block_queue.empty()) {
       block_queue.pop();
    }

    for (auto& kv : blocks) {
        auto& block = kv.second;
        block->reset();
        block_queue.push(block);
    }
}

void BlockManager::update(vec3 eye_position, bool generate_blocks)
{
    switch (generator_selection) {
        case Slow:
            terrain_generator = &terrain_generator_slow;
            break;
        case Medium:
            terrain_generator = &terrain_generator_medium;
            break;
    }

    if (!block_queue.empty()) {
        shared_ptr<Block> block = block_queue.front();
        block_queue.pop();
        terrain_generator->generateTerrainBlock(*block);
        block->finish();
    }

    for (auto& kv : blocks) {
        auto& block = kv.second;
        block->update();
    }
}

void BlockManager::newBlock(ivec3 index, int size)
{
    auto block = shared_ptr<Block>(new Block(index, size));
    block->init(terrain_renderer.pos_attrib, terrain_renderer.normal_attrib,
                terrain_renderer.ambient_occlusion_attrib);
    blocks[ivec4(index, size)] = block;
    block_queue.push(block);
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

        mat4 W_reflect = translate(vec3(block.index)) *
                         glm::translate(vec3(0, water_height, 0)) *
                         glm::scale(vec3(1.0f, -1.0f, 1.0f)) *
                         glm::translate(vec3(0, -water_height, 0)) *
                         W * scale(vec3(block.size));

        glUniform1i(terrain_renderer.water_clip_uni, false);
        glUniform1i(terrain_renderer.water_reflection_clip_uni, true);
        glUniformMatrix4fv( terrain_renderer.M_uni, 1, GL_FALSE, value_ptr(W_reflect));
        glDrawTransformFeedback(GL_TRIANGLES, block.feedback_object);
    } else {
        glUniform1i(terrain_renderer.water_clip_uni, false);
        glUniform1i(terrain_renderer.water_reflection_clip_uni, false);
        glDrawTransformFeedback(GL_TRIANGLES, block.feedback_object);
    }

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

void BlockManager::processBlockOfSize(mat4 P, mat4 V, mat4 W,
                                      ivec2float_map& water_squares,
                                      ivec3 position, int size, float alpha)
{
    ivec4 index = vec4(position, size);
    if (blocks.count(index) == 0) {
        newBlock(position, size);
    } else if (blocks[index]->isReady()) {
        renderBlock(P, V, W, *blocks[index], alpha);

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

void BlockManager::renderBlocks(mat4 P, mat4 V, mat4 W, vec3 eye_position)
{
    lod.generateForPosition(P, V, W, eye_position);

    // We need to make sure not to draw water multiple times on the same grid
    // cell, because the overlapping will cause visual artifacts.
    // Keep track of the highest alpha at that cell.
    ivec2float_map needed_water_squares;

    terrain_renderer.renderer_shader.enable();
        glUniformMatrix4fv(terrain_renderer.P_uni, 1, GL_FALSE, value_ptr(P));
        glUniformMatrix4fv(terrain_renderer.V_uni, 1, GL_FALSE, value_ptr(V));

        glUniform1i(terrain_renderer.triplanar_colors_uni, triplanar_colors);
        glUniform1i(terrain_renderer.use_ambient_uni, use_ambient);
        glUniform1i(terrain_renderer.use_normal_map_uni, use_normal_map);
        glUniform1i(terrain_renderer.debug_flag_uni, debug_flag);

        glUniform1f(terrain_renderer.clip_height_uni, water_height);

        glUniform3f(terrain_renderer.eye_position_uni, eye_position.x, eye_position.y, eye_position.z);
        glUniform3f(terrain_renderer.light_position_uni, 30.0f, 50.0f, light_x);

        terrain_renderer.prepareRender();

        glEnable(GL_CLIP_DISTANCE0);

        if (large_blocks) {
            for (auto& block : lod.blocks_of_size_4) {
                if (!one_block_only) {
                    processBlockOfSize(P, V, W, needed_water_squares, block.first, 4, block.second);
                }
            }
        }

        if (medium_blocks) {
            for (auto& block : lod.blocks_of_size_2) {
                if (!one_block_only) {
                    processBlockOfSize(P, V, W, needed_water_squares, block.first, 2, block.second);
                }
            }
        }

        if (small_blocks) {
            for (auto& block : lod.blocks_of_size_1) {
                if (!one_block_only || block.first == ivec3(0, 0, 0)) {
                    processBlockOfSize(P, V, W, needed_water_squares, block.first, 1, block.second);
                }
            }
        }

        glDisable(GL_CLIP_DISTANCE0);

        // Draw the cubes
        // Highlight the active square.
    terrain_renderer.renderer_shader.disable();

    if (use_water) {
        for (auto& kv : needed_water_squares) {
            vec3 position = vec3(kv.first.x, 0.0, kv.first.y);
            float alpha = kv.second;
            mat4 block_transform = translate(position) * W;
            water.draw(P, V, glm::translate(vec3(0, water_height + 0.5f, 0)) * block_transform, alpha);
        }
    }

    CHECK_GL_ERRORS;
}
