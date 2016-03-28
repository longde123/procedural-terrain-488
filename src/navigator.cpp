#include "navigator.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <stdlib.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;
using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
Navigator::Navigator()
{
    rotation = 0.0f;
    rotation_vertical = 0.0f;
    distance_factor = 1.0f;
    camera_speed = 5.0f;
    mouse_down = false;
    mouse_down_with_control = false;

    wireframe = false;
    first_person_mode = false;
    show_lod = false;
    show_slicer = false;
    show_terrain = true;
    generate_blocks = true;
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

    GLint result;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &result);
    printf("Max 3D texture size: %d\n", result);

    // Call the script that will include shader code into other shaders.
    system((m_exec_dir + "/Assets/include.py").c_str());

	// Build the shaders
    block_manager.init(m_exec_dir + "/Assets/out/");
    density_slicer.init(m_exec_dir + "/Assets/out/");
    lod.init(m_exec_dir + "/Assets/out/");

	proj = glm::perspective(
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		0.01f, 1000.0f );

    resetView();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
    makeView();
}

void Navigator::resetView()
{
    float distance = 5.0 * M_SQRT1_2 * distance_factor;
    vec3 x_axis(1.0f, 0.0f, 0.0f);
    vec3 y_axis(0.0f, 1.0f, 0.0f);
    eye_position = rotate(rotate(vec3(0.0f, distance, distance), rotation_vertical, x_axis), rotation, y_axis);
    eye_direction = -normalize(eye_position);

    eye_up = vec3(0.0f, 1.0f, 0.0f);
    vec3 eye_right = cross(eye_direction, eye_up);
    eye_up = cross(eye_right, eye_direction);
}

void Navigator::makeView()
{
    view = lookAt(eye_position, eye_position + eye_direction, eye_up);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void Navigator::appLogic()
{
    // Animations should not be dependent on FPS.
    float time_elapsed = ImGui::GetIO().DeltaTime;

    // First person camera controls.
    if (first_person_mode) {
        vec3 eye_right = cross(eye_direction, eye_up);
        float factor = 0.02f * camera_speed;

        // Left
        if (pressed_keys.find(GLFW_KEY_A) != pressed_keys.end()) {
            eye_position -= eye_right * factor;
        }
        // Right
        if (pressed_keys.find(GLFW_KEY_D) != pressed_keys.end()) {
            eye_position += eye_right * factor;
        }
        // Back
        if (pressed_keys.find(GLFW_KEY_S) != pressed_keys.end()) {
            eye_position -= eye_direction * factor;
        }
        // Forward
        if (pressed_keys.find(GLFW_KEY_W) != pressed_keys.end()) {
            eye_position += eye_direction * factor;
        }
        // Up
        if (pressed_keys.find(GLFW_KEY_Q) != pressed_keys.end()) {
            eye_position += eye_up * factor;
        }
        // Down
        if (pressed_keys.find(GLFW_KEY_E) != pressed_keys.end()) {
            eye_position -= eye_up * factor;
        }

        makeView();
    }

    block_manager.update(time_elapsed, eye_position, generate_blocks);
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

    // Without this, menu won't be visible.
    windowFlags |= ImGuiWindowFlags_MenuBar;

	if (ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags)) {

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Wireframe", NULL, &wireframe)) {}
                if (ImGui::MenuItem("Triplanar Colors", NULL, &block_manager.triplanar_colors)) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::SliderFloat("Period", &block_manager.terrain_generator->period, 10.0f, 100.0f)) {
            // Need to regenerate terrain.
            block_manager.regenerateAllBlocks(false);
        }

        if (ImGui::SliderFloat("Light X", &block_manager.light_x, 0.0f, 70.0f)) {
        }
        if (ImGui::SliderFloat("Water Height", &block_manager.water_height, -0.5f, 1.5f)) {
        }
        if (ImGui::SliderFloat("Camera Speed", &camera_speed, 0.5f, 20.0f)) {
        }

        if (ImGui::Checkbox("First Person Mode", &first_person_mode)) {
            resetView();
            makeView();
        }
        ImGui::Checkbox("Show Level of Detail", &show_lod);
        ImGui::Checkbox("Show Slicer", &show_slicer);
        ImGui::Checkbox("Show Terrain", &show_terrain);
        ImGui::Checkbox("Generate Blocks", &generate_blocks);
        ImGui::Checkbox("Ambient Occlusion", &block_manager.use_ambient);
        ImGui::Checkbox("Normal Maps", &block_manager.use_normal_map);
        ImGui::Checkbox("Use Water", &block_manager.use_water);
        ImGui::Checkbox("Debug Flags", &block_manager.debug_flag);
        ImGui::Checkbox("Small Blocks", &block_manager.small_blocks);
        ImGui::Checkbox("Medium Blocks", &block_manager.medium_blocks);
        ImGui::Checkbox("Large Blocks", &block_manager.large_blocks);
        if (ImGui::Checkbox("Short Range Ambient Occlusion",
                    &block_manager.terrain_generator->use_short_range_ambient_occlusion)) {
            block_manager.regenerateAllBlocks();
        }
        if (ImGui::Checkbox("Long Range Ambient Occlusion",
                    &block_manager.terrain_generator->use_long_range_ambient_occlusion)) {
            block_manager.regenerateAllBlocks();
        }

        if (ImGui::RadioButton("Slow Generator", (int*)&block_manager.generator_selection, 0)) {}
        if (ImGui::RadioButton("Medium Generator", (int*)&block_manager.generator_selection, 1)) {}

        if (ImGui::RadioButton("One Block", (int*)&block_manager.block_display_type, 0)) {}
        if (ImGui::RadioButton("Eight Blocks", (int*)&block_manager.block_display_type, 1)) {}
        if (ImGui::RadioButton("All", (int*)&block_manager.block_display_type, 2)) {}

		if (ImGui::Button("Profile Block Generation")) {
            block_manager.profileBlockGeneration();
		}

		if (ImGui::Button("Quit Application")) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Text("Blocks to render: %d", block_manager.blocksInQueue());
    }
	ImGui::End();

	if (showTestWindow) {
		ImGui::ShowTestWindow(&showTestWindow);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void Navigator::draw()
{
	// Create a global transformation for the model.
	mat4 W;
    float offset = -0.5f;
	W = glm::translate(W, vec3(offset, offset, offset));

    if (wireframe) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }

    glEnable( GL_DEPTH_TEST );

    if (show_slicer) {
        density_slicer.draw(proj, view, W, block_manager.terrain_generator->period);
    }

    if (show_terrain) {
        block_manager.renderBlocks(proj, view, W, eye_position);
    }

    if (show_lod) {
        lod.draw(proj, view, W, eye_position);
    }

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
            float dy = yPos - previous_mouse_y;
            float dx = xPos - previous_mouse_x;

            if (first_person_mode) {
                vec3 eye_right = cross(eye_direction, eye_up);
                eye_direction = rotate(eye_direction, dy / 500.0f, eye_right);
                eye_up = rotate(eye_up, dy / 500.0f, eye_right);

                eye_direction = rotate(eye_direction, dx / 500.0f, vec3(0.0, 1.0, 0.0));
                eye_up = rotate(eye_up, dx / 500.0f, vec3(0.0, 1.0, 0.0));

                eye_direction = normalize(eye_direction);
                eye_up = normalize(eye_up);

                // TODO: Might want to prevent looking too far up or down.
                // http://gamedev.stackexchange.com/questions/19507/how-should-i-implement-a-first-person-camera

                makeView();
            } else {
                if (mouse_down_with_control) {
                    rotation_vertical += -dy / 500.0f;
                    rotation_vertical = std::max(std::min(rotation_vertical, PI / 4.0f), -PI / 4.0f);
                    resetView();
                    makeView();
                } else {
                    rotation += -dx / 500.0f;
                    resetView();
                    makeView();
                }
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
    distance_factor *= exp(-yOffSet / 10.0f);
    // Put reasonable bounds.
    distance_factor = std::max(0.1f, std::min(100.0f, distance_factor));

    resetView();
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

	if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(m_window, GL_TRUE);

            eventHandled = true;
        }
        if (key == GLFW_KEY_F) {
            block_manager.debug_flag = !block_manager.debug_flag;

            eventHandled = true;
        }
        if (key == GLFW_KEY_N) {
            block_manager.use_normal_map = !block_manager.use_normal_map;

            eventHandled = true;
        }
        if (key == GLFW_KEY_G) {
            generate_blocks = !generate_blocks;

            eventHandled = true;
        }

        pressed_keys.insert(key);
	}

	if (action == GLFW_RELEASE) {
        pressed_keys.erase(key);
	}

	return eventHandled;
}
