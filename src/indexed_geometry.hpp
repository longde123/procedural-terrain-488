#pragma once

#include "geometry.hpp"

#include <vector>

class IndexedGeometry : public Geometry
{
public:
    GLuint getIndices() { return indices_buffer; }
    int indexCount() { return indices.size(); }

protected:
    GLuint indices_buffer;
    std::vector<unsigned int> indices;

    void initFromVertices(ShaderProgram& shader_program, float* vertices, int vertex_count);
};
