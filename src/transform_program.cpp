#include "transform_program.hpp"

#include <assert.h>
#include <vector>

#include <glm/glm.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

TransformProgram::TransformProgram(const GLchar** varyings, int count)
: varyings(varyings), count(count)
{
}

void TransformProgram::link()
{
    attachShaders();

    glTransformFeedbackVaryings(programObject, count, varyings, GL_INTERLEAVED_ATTRIBS);

    glLinkProgram(programObject);
    checkLinkStatus();

    CHECK_GL_ERRORS;
}
