#include "water.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

Water::Water()
: water_plane(1.0f)
{
}

void Water::init(string dir)
{
    water_shader.generateProgramObject();
    water_shader.attachVertexShader((dir + "ColorShader.vs").c_str());
    water_shader.attachFragmentShader((dir + "ColorShader.fs").c_str());
    water_shader.link();

    P_uni = water_shader.getUniformLocation("P");
    V_uni = water_shader.getUniformLocation("V");
    M_uni = water_shader.getUniformLocation("M");
    color_uni = water_shader.getUniformLocation("color");
    eye_position_uni = water_shader.getUniformLocation("eye_position");
    fog_uni = water_shader.getUniformLocation("fog_params");

    water_plane.init(water_shader, translate(mat4(), vec3(0.5f, 0.0f, 0.5f)));

    CHECK_GL_ERRORS;
}

void Water::draw(mat4 P, mat4 V, mat4 M, vec3 eye_position, float alpha)
{
    glUniformMatrix4fv(P_uni, 1, GL_FALSE, value_ptr(P));
    glUniformMatrix4fv(V_uni, 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(M));
    glUniform4f(color_uni, 0.0f, 0.0f, 1.0f, 0.5 * alpha);

    glUniform3f(eye_position_uni, eye_position.x, eye_position.y, eye_position.z);
    glUniform3f(fog_uni, FOG_MULTIPLIER, VIEW_RANGE, FOG_BIAS);

    glBindVertexArray(water_plane.getVertices());
    glDrawArrays(GL_TRIANGLES, 0, 6);

    CHECK_GL_ERRORS;
}

void Water::start()
{
    water_shader.enable();
}

void Water::end()
{
    water_shader.disable();
}
