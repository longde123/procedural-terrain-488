#pragma once

#include "terrain_generator.hpp"

class TerrainGeneratorMedium : public TerrainGenerator {
public:
    TerrainGeneratorMedium();
    virtual ~TerrainGeneratorMedium() {}

    void init(std::string dir);

    virtual void generateTerrainBlock();

private:
    void initPackedStorage();

    TransformProgram voxel_edges_shader;
    TransformProgram triangle_unpack_shader;

    GLuint voxel_edges_feedback;

    GLint block_size_uni;
    GLint period_uni_marching;
    GLint short_range_ambient_uni;
    GLint long_range_ambient_uni;

    GLint packed_attrib;

    GLuint packed_triangles_vao;
    GLuint packed_triangles_vbo;

    // Vertices corresponding to grid points used for the geometry shader.
    Grid grid;
};
