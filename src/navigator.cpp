#include "navigator.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;
using namespace std;

static const float PI = 3.14159265f;

//----------------------------------------------------------------------------------------
// Constructor
Navigator::Navigator()
{
    rotation = 0.0f;
    rotation_vertical = 0.0f;
    distance_factor = 1.0f;
    mouse_down = false;
    mouse_down_with_control = false;
}

//----------------------------------------------------------------------------------------
// Destructor
Navigator::~Navigator()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void Navigator::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shaders
    terrain_renderer.init(m_exec_dir + "/Assets/");
    terrain_generator.init(m_exec_dir + "/Assets/");

	initGrid();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
    makeView();
	proj = glm::perspective(
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void Navigator::makeView()
{
    vec3 x_axis(1.0f, 0.0f, 0.0f);
    vec3 y_axis(0.0f, 1.0f, 0.0f);
    float distance = BLOCK_DIMENSION * 2.0 * M_SQRT1_2 * distance_factor;
    view = lookAt(
        rotate(rotate(vec3(0.0f, distance, distance), rotation_vertical, x_axis), rotation, y_axis),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f));
}

void Navigator::initGrid()
{
	size_t sz = 3 * BLOCK_DIMENSION * BLOCK_DIMENSION;

	float *verts = new float[ sz ];
	size_t ct = 0;
    for (int y = 0; y < BLOCK_DIMENSION; y++) {
        for (int x = 0; x < BLOCK_DIMENSION; x++) {
            // TODO: For optimization, try swapping x and y and see if that
            // makes any difference
            int idx = x + y * BLOCK_DIMENSION;
            verts[idx * 3] = x;
            verts[idx * 3 + 1] = 0;
            verts[idx * 3 + 2] = y;
        }
    }

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = terrain_renderer.renderer_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my*
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void Navigator::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void Navigator::guiLogic()
{
	// We already know there's only going to be one window, so for
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// Navigator running simultaneously, this would break; you'd want to make
	// this into instance fields of Navigator.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

        ImGui::SliderFloat("Period", &terrain_generator.period, 4.0f, 20.0f);

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void Navigator::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
    float offset = -BLOCK_DIMENSION / 2.0f;
	W = glm::translate(W, vec3(offset, offset, offset));

    terrain_generator.generateTerrainBlock();

	terrain_renderer.renderer_shader.enable();
		glEnable( GL_DEPTH_TEST );

        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		glUniformMatrix4fv( terrain_renderer.P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( terrain_renderer.V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( terrain_renderer.M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glDrawArraysInstanced( GL_POINTS, 0, BLOCK_DIMENSION * BLOCK_DIMENSION, BLOCK_DIMENSION );


		// Draw the cubes
		// Highlight the active square.
	terrain_renderer.renderer_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void Navigator::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool Navigator::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool Navigator::mouseMoveEvent(double xPos, double yPos)
{
    bool eventHandled(false);

    if (!ImGui::IsMouseHoveringAnyWindow()) {
        if (mouse_down) {
            if (mouse_down_with_control) {
                double dy = yPos - previous_mouse_y;
                rotation_vertical += -dy / 500.0f;
                rotation_vertical = std::max(std::min(rotation_vertical, PI / 4.1f), -PI / 4.1f);
                makeView();
            } else {
                double dx = xPos - previous_mouse_x;
                rotation += -dx / 500.0f;
                makeView();
            }
        }
    }

    previous_mouse_x = xPos;
    previous_mouse_y = yPos;

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool Navigator::mouseButtonInputEvent(int button, int actions, int mods) {
    bool eventHandled(false);

	if (actions == GLFW_PRESS) {
		if (!ImGui::IsMouseHoveringAnyWindow()) {
			mouse_down = true;

            // For rotation in the other angle.
            mouse_down_with_control = (mods & GLFW_MOD_CONTROL);
		}
	}

	if (actions == GLFW_RELEASE) {
		mouse_down = false;
	}

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool Navigator::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
    distance_factor -= yOffSet / 20.0f;
    // Put reasonable bounds.
    distance_factor = std::max(0.1f, std::min(100.0f, distance_factor));

    makeView();

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool Navigator::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool Navigator::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
        if (key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(m_window, GL_TRUE);

            eventHandled = true;
        }
	}

	return eventHandled;
}
