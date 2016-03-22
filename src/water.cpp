#include "water.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

Water::Water()
: water_plane(BLOCK_DIMENSION)
{
}

void Water::init(string dir)
{
    water_shader.generateProgramObject();
	water_shader.attachVertexShader((dir + "WaterShader.vs").c_str());
	water_shader.attachFragmentShader((dir + "WaterShader.fs").c_str());
	water_shader.link();

	P_uni = water_shader.getUniformLocation("P");
	V_uni = water_shader.getUniformLocation("V");
	M_uni = water_shader.getUniformLocation("M");

    water_plane.init(water_shader, translate(mat4(), vec3(BLOCK_DIMENSION / 2, 0.0f, BLOCK_DIMENSION / 2)));

	CHECK_GL_ERRORS;
}

void Water::draw(mat4 P, mat4 V, mat4 M)
{
    water_shader.enable();

    glUniformMatrix4fv(P_uni, 1, GL_FALSE, value_ptr(P));
    glUniformMatrix4fv(V_uni, 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(M));

    glBindVertexArray(water_plane.getVertices());
    glDrawArraysInstanced(GL_TRIANGLES, 0, BLOCK_DIMENSION * BLOCK_DIMENSION, BLOCK_DIMENSION);

    water_shader.disable();

	CHECK_GL_ERRORS;
}
