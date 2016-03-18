#include "terrain_renderer.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>

using namespace glm;
using namespace std;

TerrainRenderer::TerrainRenderer()
: texture("Textures/Stone1.JPG", GL_TEXTURE1)
, normal_map("Textures/Textures_N/Stone1_N.jpg", GL_TEXTURE2)
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

    triplanar_colors_uni = renderer_shader.getUniformLocation("triplanar_colors");
    use_ambient_uni = renderer_shader.getUniformLocation("use_ambient");

    pos_attrib = renderer_shader.getAttribLocation("position");
    normal_attrib = renderer_shader.getAttribLocation("normal");
    ambient_occlusion_attrib = renderer_shader.getAttribLocation("ambient_occlusion");

    texture.init();
    normal_map.init();

	CHECK_GL_ERRORS;
}

void TerrainRenderer::prepareRender()
{
    texture.rebind();
    normal_map.rebind();
}
