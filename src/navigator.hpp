#pragma once

#include <glm/glm.hpp>

#include <vector>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"
#include "terrain_generator.hpp"
#include "terrain_renderer.hpp"

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
	void initGrid();

    void makeView();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;

    TerrainRenderer terrain_renderer;
    TerrainGenerator terrain_generator;

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

    // Display options
    bool wireframe;

    float rotation;
    float rotation_vertical;
    float distance_factor;

    // If the main mouse is pressed.
    bool mouse_down;
    bool mouse_down_with_control;
    double previous_mouse_x;
    double previous_mouse_y;
};
