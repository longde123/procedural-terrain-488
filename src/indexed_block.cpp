#include "indexed_block.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include "timer.hpp"

using namespace glm;
using namespace std;

IndexedBlock::IndexedBlock(ivec3 index, int size, bool alpha_blend)
: Block(index, size, alpha_blend)
{
    // TODO: reevaluate amount of space needed, maybe dynamically
    vertex_unit_size = sizeof(vec3) * 2 + sizeof(float);
    vertex_data_size = BLOCK_SIZE * BLOCK_SIZE *
                       BLOCK_SIZE * vertex_unit_size * 3;
}

IndexedBlock::~IndexedBlock()
{
    // TODO
}

void IndexedBlock::init(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib)
{
    Block::init(pos_attrib, normal_attrib, ambient_occlusion_attrib);

    // TODO: reevaluate amount of space needed, maybe dynamically
    size_t index_unit_size = sizeof(ivec3);
    size_t index_data_size = BLOCK_SIZE * BLOCK_SIZE *
                             BLOCK_SIZE * index_unit_size * 15;

    glBindVertexArray(out_vao);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    {
        // TODO: Investigate performance of different usage flags.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, nullptr, GL_STATIC_COPY);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

    glGenTransformFeedbacks(1, &index_feedback);
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, index_feedback);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, index_buffer);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }

	CHECK_GL_ERRORS;
}

void IndexedBlock::draw()
{
    // Should double check the sign/unsigned thing...
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
}
