#include "indexed_geometry.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

void IndexedGeometry::initFromVertices(
        ShaderProgram& shader_program,
        float* vertices, int vertex_count)
{
    // Create the vertex array to record buffer assignments.
    glGenVertexArrays(1, &geometry_vao);
    glBindVertexArray(geometry_vao);

    // Create the cube vertex buffer
    glGenBuffers(1, &geometry_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), vertices, GL_STATIC_DRAW);

    // Specify the means of extracting the position values properly.
    GLint posAttrib = shader_program.getAttribLocation("position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(posAttrib, 0); // in case this is instanced

    // Create indices buffer
    glGenBuffers(1, &indices_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // Cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}
