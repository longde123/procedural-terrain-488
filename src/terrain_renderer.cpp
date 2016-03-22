#include "terrain_renderer.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <glm/glm.hpp>

using namespace glm;
using namespace std;

TerrainRenderer::TerrainRenderer()
: x_texture("Textures/Stone1.JPG", GL_TEXTURE1)
, y_texture("Textures/Grass2.JPG", GL_TEXTURE2)
, z_texture("Textures/Stone1.JPG", GL_TEXTURE3)
, x_normal_map("Textures/Textures_N/Stone1_N.jpg", GL_TEXTURE4)
, y_normal_map("Textures/Textures_N/Grass2_N.jpg", GL_TEXTURE5)
, z_normal_map("Textures/Textures_N/Stone1_N.jpg", GL_TEXTURE6)
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

    water_clip_uni = renderer_shader.getUniformLocation("water_clip");
    water_reflection_clip_uni = renderer_shader.getUniformLocation("water_reflection_clip");
    clip_height_uni = renderer_shader.getUniformLocation("clip_height");

    triplanar_colors_uni = renderer_shader.getUniformLocation("triplanar_colors");
    use_ambient_uni = renderer_shader.getUniformLocation("use_ambient");
    use_normal_map_uni = renderer_shader.getUniformLocation("use_normal_map");
    debug_flag_uni = renderer_shader.getUniformLocation("debug_flag");
    eye_position_uni = renderer_shader.getUniformLocation("eye_position");
    light_position_uni = renderer_shader.getUniformLocation("light_position");

    pos_attrib = renderer_shader.getAttribLocation("position");
    normal_attrib = renderer_shader.getAttribLocation("normal");
    ambient_occlusion_attrib = renderer_shader.getAttribLocation("ambient_occlusion");

    x_texture.init();
    y_texture.init();
    z_texture.init();
    x_normal_map.init();
    y_normal_map.init();
    z_normal_map.init();

	CHECK_GL_ERRORS;
}

void TerrainRenderer::prepareRender()
{
    x_texture.rebind();
    y_texture.rebind();
    z_texture.rebind();
    x_normal_map.rebind();
    y_normal_map.rebind();
    z_normal_map.rebind();
}
