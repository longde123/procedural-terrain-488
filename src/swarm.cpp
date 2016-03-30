#include "swarm.hpp"

#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

#define SWARM_SIZE 1024

Swarm::Swarm()
: cube(0.12)
{
}

void Swarm::init(string dir)
{
    initialization_shader.generateProgramObject();
    initialization_shader.attachComputeShader((dir + "SwarmInitialization.cs").c_str());
    initialization_shader.link();

    update_shader.generateProgramObject();
    update_shader.attachVertexShader((dir + "ColorShaderAttrib.vs").c_str());
    update_shader.attachFragmentShader((dir + "ColorShader.fs").c_str());
    update_shader.link();

    cube.init(update_shader);

	block_size_uni = initialization_shader.getUniformLocation("block_size");
	period_uni = initialization_shader.getUniformLocation("period");
	warp_params_uni = initialization_shader.getUniformLocation("warp_params");

	P_uni = update_shader.getUniformLocation("P");
	V_uni = update_shader.getUniformLocation("V");
    M_uni = update_shader.getUniformLocation("M");

    pos_attrib = update_shader.getAttribLocation("instance_pos");
    color_attrib = update_shader.getAttribLocation("color");

    glGenBuffers(1, &positions_buffer);
    glGenBuffers(1, &velocities_buffer);
    glGenBuffers(1, &colors_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, velocities_buffer);
    glBufferData(GL_ARRAY_BUFFER, SWARM_SIZE * sizeof(vec3), NULL, GL_STREAM_DRAW);

    glBindVertexArray(cube.getVertices());
    {
        glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
        glBufferData(GL_ARRAY_BUFFER, SWARM_SIZE * sizeof(vec3), NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
        glVertexAttribDivisor(pos_attrib, 1); // 1 per object

        glBindBuffer(GL_ARRAY_BUFFER, colors_buffer);
        glBufferData(GL_ARRAY_BUFFER, SWARM_SIZE * sizeof(vec3), NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(color_attrib);
        glVertexAttribPointer(color_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
        glVertexAttribDivisor(color_attrib, 1); // 1 per object
    }
    glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

void Swarm::draw(mat4 P, mat4 V, mat4 M, vec3 eye_position, TerrainGenerator& terrain_generator)
{
    update_shader.enable();
    {
        glBindVertexArray(cube.getVertices());

        glUniformMatrix4fv(P_uni, 1, GL_FALSE, value_ptr(P));
        glUniformMatrix4fv(V_uni, 1, GL_FALSE, value_ptr(V));
        glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(M));

        glDrawElementsInstanced(GL_TRIANGLES, cube.indexCount(), GL_UNSIGNED_INT, 0, SWARM_SIZE);

        glBindVertexArray(0);
    }
    update_shader.disable();

	CHECK_GL_ERRORS;
}

void Swarm::initializeAttributes(TerrainGenerator& terrain_generator)
{
    initialization_shader.enable();
    {
        glUniform1i(block_size_uni, BLOCK_SIZE);
        glUniform2f(warp_params_uni, terrain_generator.warp_frequency, terrain_generator.warp_strength);
        glUniform1f(period_uni, terrain_generator.period);

        glDispatchCompute(1, 1, 1);

        // Block until kernel/shader finishes execution.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    initialization_shader.disable();

	CHECK_GL_ERRORS;
}
