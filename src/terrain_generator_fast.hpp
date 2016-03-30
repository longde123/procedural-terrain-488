#pragma once

#include "terrain_generator.hpp"

class TerrainGeneratorFast : public TerrainGenerator {
public:
    TerrainGeneratorFast();
    virtual ~TerrainGeneratorFast() {}

    void init(std::string dir);

    virtual void generateTerrainBlock(Block& block);

private:
    void initUIntStorage(GLuint& vao, GLuint& vbo, GLuint& feedback, GLint attrib);
    void initVertexLookup();

    TransformProgram list_non_empties_shader;

    GLuint non_empties_feedback;
    GLuint non_empties_query;
    GLuint case_vao;
    GLuint case_vbo;

    TransformProgram voxel_unique_edges_shader;
    GLint case_attrib;
    GLuint edge_count_query;

    GLuint unique_edges_feedback;
    GLuint unique_edges_vao;
    GLuint unique_edges_vbo;

    TransformProgram unique_vertex_shader;
    GLint edge_attrib;

    GLint block_index_uni;
    GLint block_size_uni;
    GLint block_padding_uni;
    GLint period_uni_marching;
    GLint octaves_uni_marching;
    GLint octaves_decay_uni_marching;
    GLint warp_params_uni_marching;
    GLint short_range_ambient_uni;
    GLint long_range_ambient_uni;
    GLint ambient_occlusion_param_uni;

    ShaderProgram index_shader;

    GLint total_items_uni;

    TransformProgram triangle_shader;

    GLuint index_count_query;
    GLint texture_size_uni;

    // Write vertex indices for fast lookup when constructing triangles.
    GLuint lookup_texture;

    // Vertices corresponding to grid points used for the geometry shader.
    Grid grid;
};
