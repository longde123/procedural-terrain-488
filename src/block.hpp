#pragma once

#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include "constants.hpp"

class Block {
public:
    Block(glm::ivec3 index, int size);

    void init(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib);

    void update();
    void reset();
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

private:
    bool generated;
    float transparency;
};
