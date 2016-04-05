#include "terrain_generator_slow.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>

#include "timer.hpp"

using namespace glm;
using namespace std;

static const GLchar* varyings[] = { "position", "normal", "ambient_occlusion" };

TerrainGeneratorSlow::TerrainGeneratorSlow()
: TerrainGenerator()
, marching_cubes_shader(varyings, 3)
, grid(BLOCK_SIZE)
{
}

void TerrainGeneratorSlow::init(string dir)
{
    TerrainGenerator::init(dir);

    marching_cubes_shader.generateProgramObject();
    marching_cubes_shader.attachVertexShader((dir + "GridPointShader.vs").c_str());
    marching_cubes_shader.attachGeometryShader((dir + "MarchingCubesShader.gs").c_str());
    marching_cubes_shader.link();

    block_size_uni = marching_cubes_shader.getUniformLocation("block_size");
    block_padding_uni_marching = marching_cubes_shader.getUniformLocation("block_padding");
    period_uni_marching = marching_cubes_shader.getUniformLocation("period");
    octaves_uni_marching = marching_cubes_shader.getUniformLocation("octaves");
    octaves_decay_uni_marching = marching_cubes_shader.getUniformLocation("octaves_decay");
    warp_params_uni_marching = marching_cubes_shader.getUniformLocation("warp_params");
    short_range_ambient_uni = marching_cubes_shader.getUniformLocation("short_range_ambient");
    long_range_ambient_uni = marching_cubes_shader.getUniformLocation("long_range_ambient");

    grid.init(marching_cubes_shader);
}

void TerrainGeneratorSlow::generateTerrainBlock(Block& block)
{
    generateDensity(block);

    // Generate the triangle mesh for the terrain.
    marching_cubes_shader.enable();
    {
        glUniform1f(period_uni_marching, period);
        glUniform1i(octaves_uni_marching, octaves);
        glUniform1f(octaves_decay_uni_marching, octaves_decay);
        glUniform1i(block_size_uni, BLOCK_SIZE);
        glUniform1i(block_padding_uni_marching, BLOCK_PADDING);
        glUniform1f(short_range_ambient_uni, use_short_range_ambient_occlusion);
        glUniform1f(long_range_ambient_uni, use_long_range_ambient_occlusion);

        // The terrain generator just saves vertices in world space.
        glEnable(GL_RASTERIZER_DISCARD);

        glBindVertexArray(grid.getVertices());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, block.feedback_object);

        glBindBuffer(GL_ARRAY_BUFFER, block.out_vbo);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, block.out_vbo);

        glBeginTransformFeedback(GL_TRIANGLES);
        {
            glDrawArraysInstanced(GL_POINTS, 0, BLOCK_SIZE * BLOCK_SIZE, BLOCK_SIZE);
        }
        glEndTransformFeedback();

#if ONE_BLOCK_PROFILE
            Timer timer2;
            timer2.start();
            for (int i = 0; i < 100; i++) {
                glBeginTransformFeedback(GL_TRIANGLES);
                glDrawArraysInstanced(GL_POINTS, 0, BLOCK_SIZE * BLOCK_SIZE, BLOCK_SIZE);
                glEndTransformFeedback();
            }
            glFinish();
            timer2.stop();
            printf("Slow - all - %.3f\n", timer2.elapsedSeconds());
#endif

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);

        glDisable(GL_RASTERIZER_DISCARD);
    }
    marching_cubes_shader.disable();

    CHECK_GL_ERRORS;
}
