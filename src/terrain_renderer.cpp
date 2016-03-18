#include "terrain_renderer.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>

using namespace glm;
using namespace std;

TerrainRenderer::TerrainRenderer()
{
}

void TerrainRenderer::init(string dir)
{
    renderer_shader.generateProgramObject();
	renderer_shader.attachVertexShader((dir + "VertexShader.vs").c_str());
	renderer_shader.attachFragmentShader((dir + "FragmentShader.fs").c_str());
	renderer_shader.link();

	// Set up the uniforms
	P_uni = renderer_shader.getUniformLocation( "P" );
	V_uni = renderer_shader.getUniformLocation( "V" );
	M_uni = renderer_shader.getUniformLocation( "M" );
	NormalMatrix_uni = renderer_shader.getUniformLocation( "NormalMatrix" );

    posAttrib = renderer_shader.getAttribLocation("position");
    colorAttrib = renderer_shader.getAttribLocation("color");
    normalAttrib = renderer_shader.getAttribLocation("normal");

	CHECK_GL_ERRORS;
}
