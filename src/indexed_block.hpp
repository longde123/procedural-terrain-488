#pragma once

#include "block.hpp"

class IndexedBlock : public Block {
public:
    IndexedBlock(glm::ivec3 index, int size, bool alpha_blend = true);
    virtual ~IndexedBlock();

    void init(GLint pos_attrib, GLint normal_attrib, GLint ambient_occlusion_attrib);

    void draw();

    GLuint index_buffer;
    GLuint index_feedback;

    int index_count;
private:
};
