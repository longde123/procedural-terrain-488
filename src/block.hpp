#pragma once

#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include "constants.hpp"

class Block {
public:
    Block(glm::ivec3 index, int size, bool alpha_blend = true);
    virtual ~Block();

    virtual void init(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib);

    void update(float time_elapsed);
    virtual void draw();

    // Don't name this function "reset", the compiler won't catch the mistake // if the block is stored in a shared_ptr and we accidently do .reset
    // instead of ->reset.
    void resetBlock(bool alpha_blend = true);

    void finish() { generated = true; }
    bool isReady() { return generated; }
    float getAlpha() { return transparency; }

    glm::ivec3 index;

    int size;

    // TODO: Make a buffer pool to recycle these.
    GLuint out_vao;
    GLuint out_vbo;

    // Careful, these cannot be reused for multiple buffers!
    GLuint feedback_object;

protected:
    size_t vertex_unit_size;
    size_t vertex_data_size;

private:
    bool generated;
    float transparency;
};
