#include "density_slicer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

DensitySlicer::DensitySlicer()
: xy_grid(1.0)
, yz_grid(1.0)
, xz_grid(1.0)
{
}

void DensitySlicer::init(string dir)
{
    density_shader.generateProgramObject();
	density_shader.attachVertexShader((dir + "DensitySliceShader.vs").c_str());
	density_shader.attachFragmentShader((dir + "DensitySliceShader.fs").c_str());
	density_shader.link();

	P_uni = density_shader.getUniformLocation("P");
	V_uni = density_shader.getUniformLocation("V");
	M_uni = density_shader.getUniformLocation("M");
	block_index_uni = density_shader.getUniformLocation("block_index");
	block_size_uni = density_shader.getUniformLocation("block_size");
	period_uni = density_shader.getUniformLocation("period");
	octaves_uni = density_shader.getUniformLocation("octaves");
	octaves_decay_uni = density_shader.getUniformLocation("octaves_decay");
	warp_params_uni = density_shader.getUniformLocation("warp_params");

    mat4 translation = translate(mat4(), vec3(0.5f));
    xy_grid.init(density_shader, translation * rotate(mat4(), PI / 2, vec3(1.0f, 0, 0)));
    yz_grid.init(density_shader, translation * rotate(mat4(), PI / 2, vec3(0, 0, 1.0f)));
    xz_grid.init(density_shader, translation);

	CHECK_GL_ERRORS;
}

void DensitySlicer::draw(mat4 P, mat4 V, mat4 M, float size,
                         float period, int octaves, float octaves_decay,
                         float warp_frequency, float warp_strength)
{
    density_shader.enable();

    mat4 transform = M * scale(vec3(size));

    glUniformMatrix4fv(P_uni, 1, GL_FALSE, value_ptr(P));
    glUniformMatrix4fv(V_uni, 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
    glUniform4i(block_index_uni, 0, 0, 0, 2);
    glUniform1f(block_size_uni, BLOCK_SIZE);
    glUniform1f(period_uni, period);
    glUniform1i(octaves_uni, octaves);
    glUniform1f(octaves_decay_uni, octaves_decay);
    glUniform2f(warp_params_uni, warp_frequency, warp_strength);

    glBindVertexArray(xy_grid.getVertices());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(yz_grid.getVertices());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(xz_grid.getVertices());
    glDrawArrays(GL_TRIANGLES, 0, 6);

	CHECK_GL_ERRORS;

    density_shader.disable();
}
