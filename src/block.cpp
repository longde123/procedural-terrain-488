#include "block.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include "timer.hpp"

using namespace glm;
using namespace std;

Block::Block(ivec3 index, int size, bool alpha_blend)
: index(index)
, size(size)
{
    // TODO: reevaluate amount of space needed, maybe dynamically
    vertex_unit_size = sizeof(vec3) * 2 + sizeof(float);
    vertex_data_size = BLOCK_SIZE * BLOCK_SIZE *
                       BLOCK_SIZE * vertex_unit_size * 15;

    resetBlock(alpha_blend);
}

Block::~Block()
{
    // TODO
}

void Block::init(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib)
{
    glGenVertexArrays(1, &out_vao);
    glGenBuffers(1, &out_vbo);
    glBindVertexArray(out_vao);
    glBindBuffer(GL_ARRAY_BUFFER, out_vbo);
    {
        // TODO: Investigate performance of different usage flags.
        glBufferData(GL_ARRAY_BUFFER, vertex_data_size, nullptr, GL_STATIC_COPY);

        // Setup location of attributes
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE,
                vertex_unit_size, 0);
        glEnableVertexAttribArray(normal_attrib);
        glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE,
                vertex_unit_size, (void*)(sizeof(vec3)));
        glEnableVertexAttribArray(ambient_occlusion_attrib);
        glVertexAttribPointer(ambient_occlusion_attrib, 1, GL_FLOAT, GL_FALSE,
                vertex_unit_size, (void*)(sizeof(vec3) * 2));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTransformFeedbacks(1, &feedback_object);
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback_object);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, out_vbo);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }

    CHECK_GL_ERRORS;
}

void Block::update(float time_elapsed)
{
    transparency = std::min(1.0f, transparency + 0.8f * time_elapsed);
}

void Block::draw()
{
    glDrawTransformFeedback(GL_TRIANGLES, feedback_object);
}

void Block::resetBlock(bool alpha_blend)
{
    generated = false;
    if (alpha_blend) {
        // Go from transparent to opaque.
        transparency = 0.0;
    } else {
        // Skip gradual transition to being fully opaque.
        transparency = 1.0;
    }
}
