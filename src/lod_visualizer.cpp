#include "lod_visualizer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

LodVisualizer::LodVisualizer()
: cube(1.0f)
, lod(16)
{
}

void LodVisualizer::init(string dir)
{
    lod_shader.generateProgramObject();
	lod_shader.attachVertexShader((dir + "ColorShader.vs").c_str());
	lod_shader.attachFragmentShader((dir + "ColorShader.fs").c_str());
	lod_shader.link();

	P_uni = lod_shader.getUniformLocation("P");
	V_uni = lod_shader.getUniformLocation("V");
	M_uni = lod_shader.getUniformLocation("M");
	color_uni = lod_shader.getUniformLocation("color");

    cube.init(lod_shader);

	CHECK_GL_ERRORS;
}

void LodVisualizer::draw(mat4 P, mat4 V, mat4 W, vec3 current_pos)
{
    lod_shader.enable();

    glUniformMatrix4fv(P_uni, 1, GL_FALSE, value_ptr(P));
    glUniformMatrix4fv(V_uni, 1, GL_FALSE, value_ptr(V));
    glBindVertexArray(cube.getVertices());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.getIndices());

    int index_count = cube.indexCount();

    lod.generateForPosition(P, V, W, current_pos);

    printf("Visible: %ld\r",
           lod.blocks_of_size_1.size() + lod.blocks_of_size_2.size() + lod.blocks_of_size_4.size());

    for (auto& block : lod.blocks_of_size_1) {
        ivec3 index = block.first;
        mat4 transform = translate(vec3(index) + vec3(0.05)) * W * scale(vec3(0.9));
        glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
        glUniform4f(color_uni, 0.0f, 0.0f, 1.0f, 0.5 * block.second);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    }

    for (auto& block : lod.blocks_of_size_2) {
        ivec3 index = block.first;
        mat4 transform = translate(vec3(index) + vec3(0.025)) * W * scale(vec3(1.95));
        glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
        glUniform4f(color_uni, 0.0f, 1.0f, 0.0f, 0.5f * block.second);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    }

    for (auto& block : lod.blocks_of_size_4) {
        ivec3 index = block.first;
        mat4 transform = translate(vec3(index)) * W * scale(vec3(4.0));
        glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(transform));
        glUniform4f(color_uni, 1.0f, 0.0f, 0.0f, 0.5f * block.second);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    }

    lod_shader.disable();

	CHECK_GL_ERRORS;
}
