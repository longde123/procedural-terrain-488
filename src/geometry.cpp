#include "geometry.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

void Geometry::initFromVertices(ShaderProgram& shaderProgram, float* vertices, int vertex_count)
{
    // Create the vertex array to record buffer assignments.
    glGenVertexArrays(1, &geometry_vao);
    glBindVertexArray(geometry_vao);

    // Create the cube vertex buffer
    glGenBuffers(1, &geometry_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), vertices, GL_STATIC_DRAW);

    // Specify the means of extracting the position values properly.
    GLint posAttrib = shaderProgram.getAttribLocation("position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Reset state to prevent rogue code from messing with *my* stuff!
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}
