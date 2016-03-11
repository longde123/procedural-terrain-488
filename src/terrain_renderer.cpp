#include "terrain_renderer.hpp"

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
	col_uni = renderer_shader.getUniformLocation( "colour" );
}
