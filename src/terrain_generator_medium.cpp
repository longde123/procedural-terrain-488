#include "terrain_generator_medium.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>
#include <vector>

using namespace glm;
using namespace std;

static const GLchar* packed_varyings[] = { "z6_y6_x6_edge1_edge2_edge3" };
static const GLchar* triangle_varyings[] = { "position", "normal", "ambient_occlusion" };

TerrainGeneratorMedium::TerrainGeneratorMedium()
: TerrainGenerator()
, voxel_edges_shader(packed_varyings, 1)
, triangle_unpack_shader(triangle_varyings, 3)
, grid(BLOCK_SIZE)
{
}

void TerrainGeneratorMedium::init(string dir)
{
    TerrainGenerator::init(dir);

    voxel_edges_shader.generateProgramObject();
	voxel_edges_shader.attachVertexShader((dir + "GridPointShader.vs").c_str());
	voxel_edges_shader.attachGeometryShader((dir + "VoxelEdgesShader.gs").c_str());
	voxel_edges_shader.link();

    triangle_unpack_shader.generateProgramObject();
	triangle_unpack_shader.attachVertexShader((dir + "TriangleUnpackShader.vs").c_str());
	triangle_unpack_shader.attachGeometryShader((dir + "TriangleUnpackShader.gs").c_str());
	triangle_unpack_shader.link();

    block_size_uni = triangle_unpack_shader.getUniformLocation("block_size");
	period_uni_marching = triangle_unpack_shader.getUniformLocation("period");
	short_range_ambient_uni = triangle_unpack_shader.getUniformLocation("short_range_ambient");
	long_range_ambient_uni = triangle_unpack_shader.getUniformLocation("long_range_ambient");

    packed_attrib = triangle_unpack_shader.getAttribLocation("z6_y6_x6_edge1_edge2_edge3_in");

    initPackedStorage();

    grid.init(voxel_edges_shader);
}

void TerrainGeneratorMedium::initPackedStorage()
{
    size_t unit_size = sizeof(unsigned int);
    size_t data_size = BLOCK_SIZE * BLOCK_SIZE *
                       BLOCK_SIZE * unit_size * 5;

	glGenVertexArrays(1, &packed_triangles_vao);
    glGenBuffers(1, &packed_triangles_vbo);
	glBindVertexArray(packed_triangles_vao);
    glBindBuffer(GL_ARRAY_BUFFER, packed_triangles_vbo);
    {
        // TODO: Investigate performance of different usage flags.
        glBufferData(GL_ARRAY_BUFFER, data_size, nullptr, GL_DYNAMIC_COPY);

        // Setup location of attributes
        glEnableVertexAttribArray(packed_attrib);

        // Note the use of glVertexAttribIPointer instead of glVertexAttribPointer
        // which only works for floating point values but the interface still
        // lets you pass in GL_UNSIGNED_INT as type, it just screws up the bits.
        //
        // Took so long to figure out this was the problem...
        glVertexAttribIPointer(packed_attrib, 1, GL_UNSIGNED_INT, unit_size, 0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    // TODO: If we do this multiple times, make sure we clean up old copies....
    // Should probably benchmark GPU memory usage.
    glGenTransformFeedbacks(1, &voxel_edges_feedback);
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, voxel_edges_feedback);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, packed_triangles_vbo);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }

	CHECK_GL_ERRORS;
}

void TerrainGeneratorMedium::generateTerrainBlock()
{
    generateDensity();

    glEnable(GL_RASTERIZER_DISCARD);

    voxel_edges_shader.enable();
    {
        glUniform1i(block_size_uni, BLOCK_SIZE);

        glBindVertexArray(grid.getVertices());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, voxel_edges_feedback);
        glBeginTransformFeedback(GL_POINTS);
        {
            glDrawArraysInstanced(GL_POINTS, 0, BLOCK_SIZE * BLOCK_SIZE, BLOCK_SIZE);
        }

        glEndTransformFeedback();

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);
    }
    voxel_edges_shader.disable();

	CHECK_GL_ERRORS;

    triangle_unpack_shader.enable();
    {
        glUniform1f(period_uni_marching, period);
        glUniform1i(block_size_uni, BLOCK_SIZE);
        glUniform1f(short_range_ambient_uni, use_short_range_ambient_occlusion);
        glUniform1f(long_range_ambient_uni, use_long_range_ambient_occlusion);

        glBindVertexArray(packed_triangles_vao);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback_object);
        glBeginTransformFeedback(GL_TRIANGLES);
        {
            glDrawTransformFeedback(GL_POINTS, voxel_edges_feedback);
        }
        glEndTransformFeedback();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }
    triangle_unpack_shader.disable();

    glDisable(GL_RASTERIZER_DISCARD);

	CHECK_GL_ERRORS;
}
