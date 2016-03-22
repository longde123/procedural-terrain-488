#include "terrain_generator.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <assert.h>
#include <vector>

#include <glm/glm.hpp>

using namespace glm;
using namespace std;

#define LOCAL_DIM_X 32
#define LOCAL_DIM_Y 32
#define LOCAL_DIM_Z 1

TerrainGenerator::TerrainGenerator()
: period(20.0f)
, grid(DENSITY_BLOCK_DIMENSION)
, use_short_range_ambient_occlusion(true)
, use_long_range_ambient_occlusion(false)
{
    assert(DENSITY_BLOCK_DIMENSION % LOCAL_DIM_X == 0);
    assert(DENSITY_BLOCK_DIMENSION % LOCAL_DIM_Y == 0);
    assert(DENSITY_BLOCK_DIMENSION % LOCAL_DIM_Z == 0);
}

void TerrainGenerator::init(string dir)
{

    density_shader.generateProgramObject();
    density_shader.attachComputeShader((dir + "TerrainDensityShader.cs").c_str());
    density_shader.link();

	period_uni = density_shader.getUniformLocation("period");

    // Generate texture object in which to store the terrain block.
    glGenTextures(1, &block);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, block);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Note that we must generate a slightly larger texture than the block
    // dimension. The reason is that Marching Cubes constructs polygons out
    // of the 8 corners of a cube, so it needs to sample one grid point beyond
    // the grid size.
    glTexImage3D(GL_TEXTURE_3D,
                 0,                         // level of detail
                 GL_R32F,                   // internal format
                 DENSITY_BLOCK_DIMENSION, DENSITY_BLOCK_DIMENSION, DENSITY_BLOCK_DIMENSION,
                 0,                         // 0 is required
                 GL_RED, GL_FLOAT, NULL     // input format, not applicable
                );

    // Some people run into the issue that 3D textures need to
    // have layered be GL_TRUE
    glBindImageTexture(0,               // unit
                       block,
                       0,               // level
                       GL_TRUE,         // layered
                       0,               // layer
                       GL_READ_WRITE,   // access
                       GL_R32F);

    marching_cubes_shader.generateProgramObject();
	marching_cubes_shader.attachVertexShader((dir + "GridPointShader.vs").c_str());
	marching_cubes_shader.attachGeometryShader((dir + "MarchingCubesShader.gs").c_str());
	marching_cubes_shader.link();

	period_uni_marching = marching_cubes_shader.getUniformLocation("period");
	short_range_ambient_uni = marching_cubes_shader.getUniformLocation("short_range_ambient");
	long_range_ambient_uni = marching_cubes_shader.getUniformLocation("long_range_ambient");

    grid.init(marching_cubes_shader);
}

void TerrainGenerator::initBuffer(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib)
{
    size_t unit_size = sizeof(vec3) * 2 + sizeof(float);
    size_t data_size = DENSITY_BLOCK_DIMENSION * DENSITY_BLOCK_DIMENSION *
                       DENSITY_BLOCK_DIMENSION * unit_size * 15;

	glGenVertexArrays(1, &out_vao);
    glGenBuffers(1, &out_vbo);
	glBindVertexArray(out_vao);
    glBindBuffer(GL_ARRAY_BUFFER, out_vbo);
    {
        // TODO: Investigate performance of different usage flags.
        glBufferData(GL_ARRAY_BUFFER, data_size, nullptr, GL_DYNAMIC_COPY);

        // Setup location of attributes
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE,
                unit_size, 0);
        glEnableVertexAttribArray(normal_attrib);
        glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE,
                unit_size, (void*)(sizeof(vec3)));
        glEnableVertexAttribArray(ambient_occlusion_attrib);
        glVertexAttribPointer(ambient_occlusion_attrib, 3, GL_FLOAT, GL_FALSE,
                unit_size, (void*)(sizeof(vec3) * 2));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    glGenTransformFeedbacks(1, &feedback_object);
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback_object);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, out_vbo);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }

	CHECK_GL_ERRORS;
}

void TerrainGenerator::generateTerrainBlock()
{
    // Generate the density values for the terrain block.
    density_shader.enable();
    {
        glUniform1f(period_uni, period);

        glDispatchCompute(DENSITY_BLOCK_DIMENSION / LOCAL_DIM_X,
                          DENSITY_BLOCK_DIMENSION / LOCAL_DIM_Y,
                          DENSITY_BLOCK_DIMENSION / LOCAL_DIM_Z);

        // Block until kernel/shader finishes execution.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // For debugging, get the data out of the GPU.
        /*
        vector<float> data(BLOCK_DIMENSION * BLOCK_DIMENSION * BLOCK_DIMENSION);
        glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, &data[0]);
        for (int i = 0; i < 10; i++) {
            printf("%f\n", data[i]);
        }
        for (int i = 0; i < BLOCK_DIMENSION * BLOCK_DIMENSION * BLOCK_DIMENSION; i++) {
            if (!(data[i] >= -1.0 && data[i] <= 1.0)) {
                printf("%f\n", data[i]);
                break;
            }
        }
        */
    }
    density_shader.disable();

	CHECK_GL_ERRORS;

    // Generate the triangle mesh for the terrain.
    marching_cubes_shader.enable();
    {
        GLint block_size_uni = marching_cubes_shader.getUniformLocation("block_size");

        glUniform1f(period_uni_marching, period);
        glUniform1i(block_size_uni, DENSITY_BLOCK_DIMENSION);
        glUniform1f(short_range_ambient_uni, use_short_range_ambient_occlusion);
        glUniform1f(long_range_ambient_uni, use_long_range_ambient_occlusion);

        // The terrain generator just saves vertices in world space.
        glEnable(GL_RASTERIZER_DISCARD);

        // Just draw the grid for now.
        glBindVertexArray(grid.getVertices());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback_object);
        glBeginTransformFeedback(GL_TRIANGLES);
        {
            glDrawArraysInstanced(GL_POINTS, 0, BLOCK_DIMENSION * BLOCK_DIMENSION, BLOCK_DIMENSION);
        }
        glEndTransformFeedback();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

        /*
        // Get some data on the CPU side to see if it looks right.
        {
            glBindBuffer(GL_ARRAY_BUFFER, marching_cubes_shader.getBuffer());
            int n = 30;
            vector<float> data(n);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * n, &data[0]);
            CHECK_GL_ERRORS;
            for (int i = 0; i < n; i++) {
                printf("%f\n", data[i]);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        */

        glBindVertexArray(0);
        glDisable(GL_RASTERIZER_DISCARD);
    }
    marching_cubes_shader.disable();

	CHECK_GL_ERRORS;
}
