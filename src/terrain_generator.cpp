#include "terrain_generator.hpp"

#include <assert.h>

using namespace std;

#define LOCAL_DIM_X 32
#define LOCAL_DIM_Y 32
#define LOCAL_DIM_Z 1

TerrainGenerator::TerrainGenerator()
{
    assert(BLOCK_DIMENSION % LOCAL_DIM_X == 0);
    assert(BLOCK_DIMENSION % LOCAL_DIM_Y == 0);
    assert(BLOCK_DIMENSION % LOCAL_DIM_Z == 0);
}

void TerrainGenerator::init(string dir)
{
    terrain_shader.generateProgramObject();
    terrain_shader.attachComputeShader((dir + "TerrainDensityShader.cs").c_str());
    terrain_shader.link();
}

void TerrainGenerator::generateTerrainBlock()
{
    // Generate texture object in which to store the terrain block.
    glGenTextures(1, &block);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, block);
    glTexImage3D(GL_TEXTURE_3D,
                 0,                         // level of detail
                 GL_R32F,                   // internal format
                 BLOCK_DIMENSION, BLOCK_DIMENSION, BLOCK_DIMENSION,
                 0,                         // 0 is required
                 GL_RED, GL_FLOAT, NULL     // input format, not applicable
                );
    glBindImageTexture(0,               // unit
                       block,
                       0,               // level
                       GL_FALSE,        // layered
                       0,               // layer
                       GL_READ_WRITE,   // access
                       GL_R32F);

    // Generate the density values for the terrain block.
    terrain_shader.enable();
    glDispatchCompute(BLOCK_DIMENSION / LOCAL_DIM_X,
                      BLOCK_DIMENSION / LOCAL_DIM_Y,
                      BLOCK_DIMENSION / LOCAL_DIM_Z);

    // Block until kernel/shader finishes execution.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
