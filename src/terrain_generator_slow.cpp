#include "terrain_generator_slow.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>

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

	period_uni_marching = marching_cubes_shader.getUniformLocation("period");
	short_range_ambient_uni = marching_cubes_shader.getUniformLocation("short_range_ambient");
	long_range_ambient_uni = marching_cubes_shader.getUniformLocation("long_range_ambient");

    grid.init(marching_cubes_shader);
}

void TerrainGeneratorSlow::generateTerrainBlock()
{
    generateDensity();

    // Generate the triangle mesh for the terrain.
    marching_cubes_shader.enable();
    {
        GLint block_size_uni = marching_cubes_shader.getUniformLocation("block_size");

        glUniform1f(period_uni_marching, period);
        glUniform1i(block_size_uni, BLOCK_SIZE);
        glUniform1f(short_range_ambient_uni, use_short_range_ambient_occlusion);
        glUniform1f(long_range_ambient_uni, use_long_range_ambient_occlusion);

        // The terrain generator just saves vertices in world space.
        glEnable(GL_RASTERIZER_DISCARD);

        glBindVertexArray(grid.getVertices());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback_object);
        glBeginTransformFeedback(GL_TRIANGLES);
        {
            glDrawArraysInstanced(GL_POINTS, 0, BLOCK_SIZE * BLOCK_SIZE, BLOCK_SIZE);
        }
        glEndTransformFeedback();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

        glBindVertexArray(0);
        glDisable(GL_RASTERIZER_DISCARD);
    }
    marching_cubes_shader.disable();

	CHECK_GL_ERRORS;
}
