#include "terrain_generator_fast.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>
#include <vector>

#include "indexed_block.hpp"
#include "timer.hpp"

using namespace glm;
using namespace std;

static const GLchar* non_empties_varyings[] = { "z6_y6_x6_case8" };
static const GLchar* unique_edges_varyings[] = { "z6_y6_x6_edge4" };
static const GLchar* triangle_index_varyings[] = { "index" };
static const GLchar* triangle_vertex_varyings[] = { "position", "normal", "ambient_occlusion" };

TerrainGeneratorFast::TerrainGeneratorFast()
: TerrainGenerator()
, list_non_empties_shader(non_empties_varyings, 1)
, voxel_unique_edges_shader(unique_edges_varyings, 1)
, triangle_shader(triangle_index_varyings, 1)
, unique_vertex_shader(triangle_vertex_varyings, 3)
, grid(BLOCK_SIZE)
{
}

void TerrainGeneratorFast::init(string dir)
{
    TerrainGenerator::init(dir);

    list_non_empties_shader.generateProgramObject();
    list_non_empties_shader.attachVertexShader((dir + "GridPointShader.vs").c_str());
    list_non_empties_shader.attachGeometryShader((dir + "ListNonEmpties.gs").c_str());
    list_non_empties_shader.link();

    glGenQueries(1, &non_empties_query);

    grid.init(list_non_empties_shader);

    voxel_unique_edges_shader.generateProgramObject();
    voxel_unique_edges_shader.attachVertexShader((dir + "VoxelUniqueEdges.vs").c_str());
    voxel_unique_edges_shader.attachGeometryShader((dir + "VoxelUniqueEdges.gs").c_str());
    voxel_unique_edges_shader.link();

    case_attrib = voxel_unique_edges_shader.getAttribLocation("z6_y6_x6_case8_in");
    glGenQueries(1, &edge_count_query);

    unique_vertex_shader.generateProgramObject();
    unique_vertex_shader.attachVertexShader((dir + "UniqueVertex.vs").c_str());
    unique_vertex_shader.link();

    edge_attrib = unique_vertex_shader.getAttribLocation("z6_y6_x6_edge4_in");

    block_index_uni = unique_vertex_shader.getUniformLocation("block_index");
    block_size_uni = unique_vertex_shader.getUniformLocation("block_size");
    block_padding_uni = unique_vertex_shader.getUniformLocation("block_padding");
    period_uni_marching = unique_vertex_shader.getUniformLocation("period");
    octaves_uni_marching = unique_vertex_shader.getUniformLocation("octaves");
    octaves_decay_uni_marching = unique_vertex_shader.getUniformLocation("octaves_decay");
    warp_params_uni_marching = unique_vertex_shader.getUniformLocation("warp_params");
    short_range_ambient_uni = unique_vertex_shader.getUniformLocation("short_range_ambient");
    long_range_ambient_uni = unique_vertex_shader.getUniformLocation("long_range_ambient");
    ambient_occlusion_param_uni = unique_vertex_shader.getUniformLocation("ambient_occlusion_param");

    initUIntStorage(case_vao, case_vbo, non_empties_feedback, case_attrib);
    initUIntStorage(unique_edges_vao, unique_edges_vbo, unique_edges_feedback, edge_attrib);

    index_shader.generateProgramObject();
    index_shader.attachComputeShader((dir + "ConstructIndexLookup.cs").c_str());
    index_shader.link();

    total_items_uni = index_shader.getUniformLocation("total_items");

    triangle_shader.generateProgramObject();
    triangle_shader.attachVertexShader((dir + "VoxelUniqueEdges.vs").c_str());
    triangle_shader.attachGeometryShader((dir + "ConstructTriangles.gs").c_str());
    triangle_shader.link();

    texture_size_uni = triangle_shader.getUniformLocation("texture_size");
    glGenQueries(1, &index_count_query);

    initVertexLookup();
}

void TerrainGeneratorFast::initUIntStorage(GLuint& vao, GLuint& vbo, GLuint& feedback, GLint attrib)
{
    size_t unit_size = sizeof(unsigned int);
    size_t data_size = BLOCK_SIZE * BLOCK_SIZE *
                       BLOCK_SIZE * unit_size;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    {
        // TODO: Investigate performance of different usage flags.
        glBufferData(GL_ARRAY_BUFFER, data_size, nullptr, GL_STATIC_COPY);

        // Setup location of attributes
        glEnableVertexAttribArray(attrib);

        // Note the use of glVertexAttribIPointer instead of glVertexAttribPointer
        // which only works for floating point values but the interface still
        // lets you pass in GL_UNSIGNED_INT as type, it just screws up the bits.
        glVertexAttribIPointer(attrib, 1, GL_UNSIGNED_INT, unit_size, 0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TODO: If we do this multiple times, make sure we clean up old copies....
    // Should probably benchmark GPU memory usage.
    glGenTransformFeedbacks(1, &feedback);
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }

    CHECK_GL_ERRORS;
}

void TerrainGeneratorFast::initVertexLookup()
{
    // Generate texture object in which to store the terrain block.
    glGenTextures(1, &lookup_texture);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_3D, lookup_texture);
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
                 0,                                     // level of detail
                 GL_R32I,                               // internal format
                 3 * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE,
                 0,                                     // 0 is required
                 GL_RED_INTEGER, GL_INT, NULL           // input format, not applicable
                );

    // Some people run into the issue that 3D textures need to
    // have layered be GL_TRUE
    glBindImageTexture(7,               // unit
                       lookup_texture,
                       0,               // level
                       GL_TRUE,         // layered
                       0,               // layer
                       GL_READ_WRITE,   // access
                       GL_R32I);

    CHECK_GL_ERRORS;
}

void TerrainGeneratorFast::generateTerrainBlock(Block& block)
{
    IndexedBlock* indexed_block = dynamic_cast<IndexedBlock*>(&block);
    assert(indexed_block != NULL);
    generateDensity(block);

    glEnable(GL_RASTERIZER_DISCARD);

    GLint non_empties_count;
    list_non_empties_shader.enable();
    {
        glBindVertexArray(grid.getVertices());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, non_empties_feedback);
        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, non_empties_query);
        glBeginTransformFeedback(GL_POINTS);
        {
            glDrawArraysInstanced(GL_POINTS, 0, BLOCK_SIZE * BLOCK_SIZE, BLOCK_SIZE);
        }
        glEndTransformFeedback();

        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
        glGetQueryObjectiv(non_empties_query, GL_QUERY_RESULT, &non_empties_count);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);
    }
    list_non_empties_shader.disable();

    CHECK_GL_ERRORS;

    GLint unique_edges_count;
    voxel_unique_edges_shader.enable();
    {
        glBindVertexArray(case_vao);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, unique_edges_feedback);
        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, edge_count_query);
        glBeginTransformFeedback(GL_POINTS);
        {
            glDrawTransformFeedback(GL_POINTS, non_empties_feedback);
        }
        glEndTransformFeedback();

        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
        glGetQueryObjectiv(edge_count_query, GL_QUERY_RESULT, &unique_edges_count);

        //printf("~%d\n" ,unique_edges_count);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);
    }
    voxel_unique_edges_shader.disable();

    CHECK_GL_ERRORS;

    index_shader.enable();
    {
        glUniform1i(total_items_uni, unique_edges_count);

        //glBindBuffer(GL_ARRAY_BUFFER, case_vbo); // needed?
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, case_vbo); // needed?
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, case_vbo);

        // This should ensure unique_edges is rounded up.
        glDispatchCompute((unique_edges_count + 1023) / 1024, 1, 1);

        // Block until kernel/shader finishes execution.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // For debugging, get the data out of the GPU.
        /*
        vector<unsigned int> data(non_empties_count);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, non_empties_count * sizeof(unsigned int), &data[0]);
        for (int i = 0; i < non_empties_count; i++) {
            printf("%d ", data[i]);
        }
        printf("\n\n");
        */
    }
    index_shader.disable();

    CHECK_GL_ERRORS;

    triangle_shader.enable();
    {
        glUniform1i(texture_size_uni, BLOCK_SIZE);

        glBindVertexArray(case_vao);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, indexed_block->index_feedback);

        // Needed?
        glBindBuffer(GL_ARRAY_BUFFER, indexed_block->index_buffer);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, indexed_block->index_buffer);

        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, index_count_query);
        glBeginTransformFeedback(GL_TRIANGLES);
        {
            glDrawTransformFeedback(GL_POINTS, non_empties_feedback);
        }
        glEndTransformFeedback();

        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
        glGetQueryObjectiv(index_count_query, GL_QUERY_RESULT, &indexed_block->index_count);

        //printf("_%d\n" , indexed_block->index_count);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);

        // For debugging, get the data out of the GPU.
        vector<unsigned int> data(indexed_block->index_count);
        glBindBuffer(GL_ARRAY_BUFFER, indexed_block->index_buffer); // needed?
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, indexed_block->index_count * sizeof(unsigned int), &data[0]);
        for (int i = 0; i < non_empties_count; i++) {
            printf("%d ", data[i]);
        }
        printf("\n\n");
    }
    triangle_shader.disable();

    CHECK_GL_ERRORS;

    unique_vertex_shader.enable();
    {
        glUniform1f(period_uni_marching, period);
        glUniform1i(octaves_uni_marching, octaves);
        glUniform1f(octaves_decay_uni_marching, octaves_decay);
        glUniform4i(block_index_uni, block.index.x, block.index.y, block.index.z, block.size);
        glUniform1i(block_size_uni, BLOCK_SIZE);
        glUniform1i(block_padding_uni, BLOCK_PADDING);
        glUniform1f(short_range_ambient_uni, use_short_range_ambient_occlusion);
        glUniform1f(long_range_ambient_uni, use_long_range_ambient_occlusion);
        glUniform4f(ambient_occlusion_param_uni,
                    ambient_occlusion_param.x, ambient_occlusion_param.y,
                    ambient_occlusion_param.z, ambient_occlusion_param.w);

        glBindVertexArray(unique_edges_vao);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, block.feedback_object);

        glBindBuffer(GL_ARRAY_BUFFER, block.out_vbo);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, block.out_vbo);

        glBeginTransformFeedback(GL_POINTS);
        {
            glDrawTransformFeedback(GL_POINTS, unique_edges_feedback);
        }
        glEndTransformFeedback();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }
    unique_vertex_shader.disable();

    glDisable(GL_RASTERIZER_DISCARD);

    CHECK_GL_ERRORS;
}
