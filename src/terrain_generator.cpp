#include "terrain_generator.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <assert.h>
#include <vector>

using namespace std;

#define LOCAL_DIM_X 32
#define LOCAL_DIM_Y 32
#define LOCAL_DIM_Z 1

TerrainGenerator::TerrainGenerator()
: period(4.0f)
{
    assert((BLOCK_DIMENSION + 1) % LOCAL_DIM_X == 0);
    assert((BLOCK_DIMENSION + 1) % LOCAL_DIM_Y == 0);
    assert((BLOCK_DIMENSION + 1) % LOCAL_DIM_Z == 0);
}

void TerrainGenerator::init(string dir)
{
    terrain_shader.generateProgramObject();
    terrain_shader.attachComputeShader((dir + "TerrainDensityShader.cs").c_str());
    terrain_shader.link();

	period_uni = terrain_shader.getUniformLocation("period");

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
                 BLOCK_DIMENSION + 1, BLOCK_DIMENSION + 1, BLOCK_DIMENSION + 1,
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
}

void TerrainGenerator::generateTerrainBlock()
{
    // Generate the density values for the terrain block.
    terrain_shader.enable();
    {
        glUniform1f(period_uni, period);

        glDispatchCompute((BLOCK_DIMENSION + 1) / LOCAL_DIM_X,
                          (BLOCK_DIMENSION + 1) / LOCAL_DIM_Y,
                          (BLOCK_DIMENSION + 1) / LOCAL_DIM_Z);

        // Block until kernel/shader finishes execution.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // For debugging, get the data out of the GPU.
        /*
        vector<float> data(BLOCK_DIMENSION * BLOCK_DIMENSION * BLOCK_DIMENSION);
        glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, &data[0]);
        for (int i = 0; i < BLOCK_DIMENSION * BLOCK_DIMENSION * BLOCK_DIMENSION; i++) {
            if (!(data[i] >= -1.0 && data[i] <= 1.0)) {
                printf("%f\n", data[i]);
                break;
            }
        }*/
    }
    terrain_shader.disable();

	CHECK_GL_ERRORS;
}
