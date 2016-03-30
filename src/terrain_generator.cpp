#include "terrain_generator.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <assert.h>
#include <vector>

#include <glm/glm.hpp>

#include "timer.hpp"

using namespace glm;
using namespace std;

// NOTE: if you update this, update it on the GPU size too
#define LOCAL_DIM_X 16
#define LOCAL_DIM_Y 16
#define LOCAL_DIM_Z 4

TerrainGenerator::TerrainGenerator()
: period(60.0f)
, octaves(8)
, octaves_decay(2.35f)
, warp_frequency(0.04f)
, warp_strength(7.0f)
, use_short_range_ambient_occlusion(true)
, use_long_range_ambient_occlusion(true)
, ambient_occlusion_param(vec4(0.3f, 0.2f, 1.0f, 9.0f))
{
    assert(BLOCK_PADDED_RESOLUTION % LOCAL_DIM_X == 0);
    assert(BLOCK_PADDED_RESOLUTION % LOCAL_DIM_Y == 0);
    assert(BLOCK_PADDED_RESOLUTION % LOCAL_DIM_Z == 0);
}

void TerrainGenerator::init(string dir)
{

    density_shader.generateProgramObject();
    density_shader.attachComputeShader((dir + "TerrainDensityShader.cs").c_str());
    density_shader.link();

	block_padding_uni = density_shader.getUniformLocation("block_padding");
	period_uni = density_shader.getUniformLocation("period");
	octaves_uni = density_shader.getUniformLocation("octaves");
	octaves_decay_uni = density_shader.getUniformLocation("octaves_decay");
	warp_params_uni = density_shader.getUniformLocation("warp_params");
	block_index_uni = density_shader.getUniformLocation("block_index");

    // Generate texture object in which to store the terrain block.
    glGenTextures(1, &block_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, block_texture);
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
                 BLOCK_PADDED_RESOLUTION, BLOCK_PADDED_RESOLUTION, BLOCK_PADDED_RESOLUTION,
                 0,                         // 0 is required
                 GL_RED, GL_FLOAT, NULL     // input format, not applicable
                );

    // Some people run into the issue that 3D textures need to
    // have layered be GL_TRUE
    glBindImageTexture(0,               // unit
                       block_texture,
                       0,               // level
                       GL_TRUE,         // layered
                       0,               // layer
                       GL_READ_WRITE,   // access
                       GL_R32F);

	CHECK_GL_ERRORS;
}

void TerrainGenerator::generateDensity(Block& block)
{
    // Generate the density values for the terrain block.
    density_shader.enable();
    {
        glUniform1i(block_padding_uni, BLOCK_PADDING);
        glUniform1i(octaves_uni, octaves);
        glUniform1f(octaves_decay_uni, octaves_decay);
        glUniform2f(warp_params_uni, warp_frequency, warp_strength);
        glUniform1f(period_uni, period);
        glUniform4i(block_index_uni,
                    block.index.x, block.index.y,
                    block.index.z, block.size);

        glDispatchCompute(BLOCK_PADDED_RESOLUTION / LOCAL_DIM_X,
                          BLOCK_PADDED_RESOLUTION / LOCAL_DIM_Y,
                          BLOCK_PADDED_RESOLUTION / LOCAL_DIM_Z);

        // Block until kernel/shader finishes execution.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

#if ONE_BLOCK_PROFILE
            Timer timer2;
            timer2.start();
            for (int i = 0; i < 100; i++) {
                glDispatchCompute(BLOCK_PADDED_RESOLUTION / LOCAL_DIM_X,
                                  BLOCK_PADDED_RESOLUTION / LOCAL_DIM_Y,
                                  BLOCK_PADDED_RESOLUTION / LOCAL_DIM_Z);

                // Block until kernel/shader finishes execution.
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            timer2.stop();
            printf("Generate block - %.3f\n", timer2.elapsedSeconds());
#endif

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
}
