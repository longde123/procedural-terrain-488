#pragma once

#include <string>

#include "cs488-framework/ShaderProgram.hpp"
#include "texture.hpp"

class TerrainRenderer {
public:
    TerrainRenderer();

    void init(std::string dir);
    void prepareRender();

    ShaderProgram renderer_shader;

	GLint P_uni;    // Uniform location for Projection matrix.
	GLint V_uni;    // Uniform location for View matrix.
	GLint M_uni;    // Uniform location for Model matrix.
	GLint NormalMatrix_uni;     // Uniform location for Normal matrix.
    GLint triplanar_colors_uni;
    GLint use_ambient_uni;

    GLint pos_attrib;
    GLint normal_attrib;
    GLint ambient_occlusion_attrib;

private:
    Texture x_texture;
    Texture y_texture;
    Texture z_texture;
    Texture x_normal_map;
    Texture y_normal_map;
    Texture z_normal_map;
};
