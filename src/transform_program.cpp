#include "transform_program.hpp"

#include <assert.h>
#include <vector>

#include <glm/glm.hpp>

#include "cs488-framework/GlErrorCheck.hpp"
using namespace glm;
using namespace std;

TransformProgram::TransformProgram()
{
}

void TransformProgram::link()
{
    attachShaders();

    static const GLchar* varyings[] = { "position", "normal", "ambient_occlusion" };
    glTransformFeedbackVaryings(programObject, 3, varyings, GL_INTERLEAVED_ATTRIBS);

    glLinkProgram(programObject);
    checkLinkStatus();

    CHECK_GL_ERRORS;
}
