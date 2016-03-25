#include "block_manager.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

BlockManager::BlockManager()
: lod(16)
{
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(0, 0, 0), 1)));
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(1, 0, 0), 1)));
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(0, 0, 1), 1)));
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(1, 0, 1), 4)));
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(-1, 0, 0), 1)));
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(0, 0, -1), 1)));
    blocks.push_back(shared_ptr<Block>(new Block(ivec3(-2, 0, -2), 2)));

    triplanar_colors = false;
    use_ambient = true;
    use_normal_map = true;
    debug_flag = false;
    use_water = true;
    water_height = 0.0f;
    light_x = 0.0f;
}

void BlockManager::init(string dir)
{
    terrain_renderer.init(dir);
    terrain_generator.init(dir);
    water.init(dir);

    for (shared_ptr<Block>& block : blocks) {
        block->init(terrain_renderer.pos_attrib,
            terrain_renderer.normal_attrib,
            terrain_renderer.ambient_occlusion_attrib);
        block_queue.push(block);
    }
}

void BlockManager::regenerateAllBlocks()
{
    // Get rid of existing stuff, we're interrupting.
    while (!block_queue.empty()) {
       block_queue.pop();
    }

    for (shared_ptr<Block>& block : blocks) {
        block->reset();
        block_queue.push(block);
    }
}

void BlockManager::update()
{
    if (!block_queue.empty()) {
        shared_ptr<Block> block = block_queue.front();
        block_queue.pop();
        terrain_generator.generateTerrainBlock(*block);
        block->finish();
    }

    for (auto& block : blocks) {
        block->update();
    }
}

void BlockManager::renderBlock(mat4 P, mat4 V, mat4 W, Block& block)
{
    mat4 block_transform = translate(vec3(block.index)) * W * scale(vec3(block.size));
    mat3 normalMatrix = mat3(transpose(inverse(block_transform)));
    glUniformMatrix4fv(terrain_renderer.M_uni, 1, GL_FALSE, value_ptr(block_transform));
    glUniformMatrix3fv(terrain_renderer.NormalMatrix_uni, 1, GL_FALSE, value_ptr(normalMatrix));

    glUniform1f(terrain_renderer.alpha_uni, block.getAlpha());

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

void BlockManager::renderBlocks(mat4 P, mat4 V, mat4 W, vec3 eye_position)
{
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

        for (auto& block : blocks) {
            // Skip blocks that are still in the queue.
            if (block->isReady()) {
                renderBlock(P, V, W, *block);
            }
        }

        glDisable(GL_CLIP_DISTANCE0);

        // Draw the cubes
        // Highlight the active square.
    terrain_renderer.renderer_shader.disable();

    if (use_water) {
        for (auto& block : blocks) {
            // Skip blocks that are still in the queue.
            if (!block->isReady()) {
                continue;
            }

            mat4 block_transform = translate(vec3(block->index)) * W * scale(vec3(block->size));
            water.draw(P, V, glm::translate(vec3(0, water_height + 0.5f, 0)) * block_transform,
                       block->getAlpha());
        }
    }

    CHECK_GL_ERRORS;
}
