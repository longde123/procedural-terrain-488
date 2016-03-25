#include "block.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

Block::Block(ivec3 index, int size)
: index(index)
, size(size)
, generated(false)
{
}

void Block::init(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib)
{
    // TODO: reevaluate amount of space needed, maybe dynamically
    size_t unit_size = sizeof(vec3) * 2 + sizeof(float);
    size_t data_size = BLOCK_SIZE * BLOCK_SIZE *
                       BLOCK_SIZE * unit_size * 15;

	glGenVertexArrays(1, &out_vao);
    glGenBuffers(1, &out_vbo);
	glBindVertexArray(out_vao);
    glBindBuffer(GL_ARRAY_BUFFER, out_vbo);
    {
        // TODO: Investigate performance of different usage flags.
        glBufferData(GL_ARRAY_BUFFER, data_size, nullptr, GL_STATIC_COPY);

        // Setup location of attributes
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE,
                unit_size, 0);
        glEnableVertexAttribArray(normal_attrib);
        glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE,
                unit_size, (void*)(sizeof(vec3)));
        glEnableVertexAttribArray(ambient_occlusion_attrib);
        glVertexAttribPointer(ambient_occlusion_attrib, 1, GL_FLOAT, GL_FALSE,
                unit_size, (void*)(sizeof(vec3) * 2));
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
