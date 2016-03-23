#pragma once

#include <glm/glm.hpp>

#include <unordered_set>
#include <vector>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "density_slicer.hpp"
#include "lod_visualizer.hpp"
#include "terrain_generator_slow.hpp"
#include "terrain_generator_medium.hpp"
#include "terrain_renderer.hpp"
#include "water.hpp"

class Navigator : public CS488Window {
public:
	Navigator();
	virtual ~Navigator();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
    void resetView();
    void makeView();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;

    DensitySlicer density_slicer;
    TerrainRenderer terrain_renderer;
    TerrainGeneratorMedium terrain_generator;
    Water water;
    LodVisualizer lod;

	// Matrices controlling the camera and projection.
    glm::vec3 eye_position;
    glm::vec3 eye_direction;
    glm::vec3 eye_up;
	glm::mat4 proj;
	glm::mat4 view;

    // First person stuff
    std::unordered_set<int> pressed_keys;

    // Display options
    bool wireframe;
    bool triplanar_colors;
    bool first_person_mode;
    bool show_lod;
    bool show_slicer;
    bool show_terrain;
    bool use_ambient;
    bool use_normal_map;
    bool use_water;
    bool debug_flag;
    float light_x;
    float water_height;

    float rotation;
    float rotation_vertical;
    float distance_factor;

    // If the main mouse is pressed.
    bool mouse_down;
    bool mouse_down_with_control;
    double previous_mouse_x;
    double previous_mouse_y;
};
