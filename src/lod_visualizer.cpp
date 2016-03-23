#include "lod_visualizer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

// Range of distance for blocks of size 1 where they fade out.
const int BLOCK_1_FADEOUT_START = 5;
const int BLOCK_1_FADEOUT_END = 6;

// Range of distance for blocks of size 2 where they fade out.
const int BLOCK_2_FADEOUT_START = 12;
const int BLOCK_2_FADEOUT_END = 14;

LodVisualizer::LodVisualizer()
: cube(1.0f)
{
}

void LodVisualizer::genSubblocks(vector<ivec3>& subblocks, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                subblocks.push_back(ivec3(i, j, k));
            }
        }
    }
}

void LodVisualizer::init(string dir)
{
    lod_shader.generateProgramObject();
	lod_shader.attachVertexShader((dir + "ColorShader.vs").c_str());
	lod_shader.attachFragmentShader((dir + "ColorShader.fs").c_str());
	lod_shader.link();

	P_uni = lod_shader.getUniformLocation("P");
	V_uni = lod_shader.getUniformLocation("V");
	M_uni = lod_shader.getUniformLocation("M");
	color_uni = lod_shader.getUniformLocation("color");

    cube.init(lod_shader);

    genSubblocks(block_2_subblocks, 2);
    genSubblocks(block_4_subblocks, 4);

	CHECK_GL_ERRORS;
}

void LodVisualizer::draw(mat4 P, mat4 V, vec3 current_pos)
{
    lod_shader.enable();

    glUniformMatrix4fv(P_uni, 1, GL_FALSE, value_ptr(P));
    glUniformMatrix4fv(V_uni, 1, GL_FALSE, value_ptr(V));
    glBindVertexArray(cube.getVertices());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.getIndices());

    int index_count = cube.indexCount();

    vector<ivec3> blocks_of_size_1;
    vector<ivec3> blocks_of_size_2;
    vector<ivec3> blocks_of_size_4;

    int range = 16;

    for (int x = -range; x <= range; x += 1) {
        for (int y = -range; y <= range; y += 1) {
            ivec3 block = ivec3(x, 0, y) + ivec3(current_pos.x, 0, current_pos.z);

            float distance = length(vec3(block) - current_pos);

            if (distance < BLOCK_1_FADEOUT_END) {
                mat4 transform = translate(vec3(block) + vec3(0.05)) * scale(vec3(0.9));
                glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
                glUniform4f(color_uni, 0.0f, 0.0f, 1.0f, 0.5f - 0.5f *
                        clamp((distance - BLOCK_1_FADEOUT_START) /
                              (BLOCK_1_FADEOUT_END - BLOCK_1_FADEOUT_START),
                              0.0f, 1.0f));
                glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
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
                mat4 transform = translate(vec3(block) + vec3(0.025)) * scale(vec3(1.95));
                glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
                glUniform4f(color_uni, 0.0f, 1.0f, 0.0f, 0.5f - 0.5f *
                        clamp((distance - BLOCK_2_FADEOUT_START) /
                              (BLOCK_2_FADEOUT_END - BLOCK_2_FADEOUT_START),
                              0.0f, 1.0f));
                glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
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
                mat4 transform = translate(vec3(block)) * scale(vec3(4.0));
                glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
                glUniform4f(color_uni, 1.0f, 0.0f, 0.0f, 0.5f);
                glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            }
        }
    }

    lod_shader.disable();

	CHECK_GL_ERRORS;
}
