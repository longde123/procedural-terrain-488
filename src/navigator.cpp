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
    mouse_down = false;
    mouse_down_with_control = false;

    wireframe = false;
    triplanar_colors = false;
    first_person_mode = false;
    show_lod = false;
    show_slicer = false;
    show_terrain = true;
    use_ambient = true;
    use_normal_map = true;
    use_water = true;
    debug_flag = false;
    light_x = 0.0f;
    water_height = 0.0f;
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
    density_slicer.init(m_exec_dir + "/Assets/out/");
    terrain_renderer.init(m_exec_dir + "/Assets/out/");
    terrain_generator.init(m_exec_dir + "/Assets/out/");
    water.init(m_exec_dir + "/Assets/out/");
    lod.init(m_exec_dir + "/Assets/out/");

    terrain_generator.initBuffer(terrain_renderer.pos_attrib,
            terrain_renderer.normal_attrib,
            terrain_renderer.ambient_occlusion_attrib);
    terrain_generator.generateTerrainBlock();

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
    float distance = 2.0 * M_SQRT1_2 * distance_factor;
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
    // First person camera controls.
    if (first_person_mode) {
        vec3 eye_right = cross(eye_direction, eye_up);
        float factor = 0.02f;

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
		if (ImGui::Button("Quit Application")) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Wireframe", NULL, &wireframe)) {}
                if (ImGui::MenuItem("Triplanar Colors", NULL, &triplanar_colors)) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::SliderFloat("Period", &terrain_generator.period, 4.0f, 40.0f)) {
            // Need to regenerate terrain.
            terrain_generator.generateTerrainBlock();
        }

        if (ImGui::SliderFloat("Light X", &light_x, 0.0f, 70.0f)) {
        }
        if (ImGui::SliderFloat("Water Height", &water_height, -0.5f, 0.5f)) {
        }

        if (ImGui::Checkbox("First Person Mode", &first_person_mode)) {
            resetView();
            makeView();
        }
        ImGui::Checkbox("Show Level of Detail", &show_lod);
        ImGui::Checkbox("Show Slicer", &show_slicer);
        ImGui::Checkbox("Show Terrain", &show_terrain);
        ImGui::Checkbox("Ambient Occlusion", &use_ambient);
        ImGui::Checkbox("Normal Maps", &use_normal_map);
        ImGui::Checkbox("Use Water", &use_water);
        ImGui::Checkbox("Debug Flags", &debug_flag);
        if (ImGui::Checkbox("Short Range Ambient Occlusion",
                    &terrain_generator.use_short_range_ambient_occlusion)) {
            terrain_generator.generateTerrainBlock();
        }
        if (ImGui::Checkbox("Long Range Ambient Occlusion",
                    &terrain_generator.use_long_range_ambient_occlusion)) {
            terrain_generator.generateTerrainBlock();
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
	// Create a global transformation for the model (centre it).
	mat4 W;
    float offset = -0.5f;
	W = glm::translate(W, vec3(offset, offset, offset));
    mat4 W_reflect = glm::translate(vec3(0, water_height, 0)) *
                     glm::scale(vec3(1.0f, -1.0f, 1.0f)) *
                     glm::translate(vec3(0, -water_height, 0)) *
                     W;

    if (wireframe) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }

    glEnable( GL_DEPTH_TEST );

    if (show_slicer) {
        density_slicer.draw(proj, view, W, terrain_generator.period);
    }

    mat3 normalMatrix = mat3(transpose(inverse(W)));

    if (show_terrain) {
        terrain_renderer.renderer_shader.enable();
            glUniformMatrix4fv( terrain_renderer.P_uni, 1, GL_FALSE, value_ptr( proj ) );
            glUniformMatrix4fv( terrain_renderer.V_uni, 1, GL_FALSE, value_ptr( view ) );
            glUniformMatrix4fv( terrain_renderer.M_uni, 1, GL_FALSE, value_ptr( W ) );
            glUniformMatrix3fv( terrain_renderer.NormalMatrix_uni, 1, GL_FALSE, value_ptr( normalMatrix ) );

            glUniform1i(terrain_renderer.triplanar_colors_uni, triplanar_colors);
            glUniform1i(terrain_renderer.use_ambient_uni, use_ambient);
            glUniform1i(terrain_renderer.use_normal_map_uni, use_normal_map);
            glUniform1i(terrain_renderer.debug_flag_uni, debug_flag);

            glUniform1f(terrain_renderer.clip_height_uni, water_height);

            glUniform3f(terrain_renderer.eye_position_uni, eye_position.x, eye_position.y, eye_position.z);
            glUniform3f(terrain_renderer.light_position_uni, 30.0f, 50.0f, light_x);

            terrain_renderer.prepareRender();

            glBindVertexArray(terrain_generator.getVertices());

            glEnable(GL_CLIP_DISTANCE0);

            if (use_water) {
                glUniform1i(terrain_renderer.water_clip_uni, true);
                glUniform1i(terrain_renderer.water_reflection_clip_uni, false);
                glDrawTransformFeedback(GL_TRIANGLES, terrain_generator.feedback_object);

                glUniform1i(terrain_renderer.water_clip_uni, false);
                glUniform1i(terrain_renderer.water_reflection_clip_uni, true);
                glUniformMatrix4fv( terrain_renderer.M_uni, 1, GL_FALSE, value_ptr(W_reflect));
                glDrawTransformFeedback(GL_TRIANGLES, terrain_generator.feedback_object);
            } else {
                glUniform1i(terrain_renderer.water_clip_uni, false);
                glUniform1i(terrain_renderer.water_reflection_clip_uni, false);
                glDrawTransformFeedback(GL_TRIANGLES, terrain_generator.feedback_object);
            }

            glDisable(GL_CLIP_DISTANCE0);

            // Draw the cubes
            // Highlight the active square.
        terrain_renderer.renderer_shader.disable();

        if (use_water) {
            water.draw(proj, view, glm::translate(vec3(0, water_height + 0.5f, 0)) * W);
        }
    }

    if (show_lod) {
        lod.draw(proj, view, eye_position);
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
            debug_flag = !debug_flag;

            eventHandled = true;
        }
        if (key == GLFW_KEY_N) {
            use_normal_map = !use_normal_map;

            eventHandled = true;
        }

        pressed_keys.insert(key);
	}

	if (action == GLFW_RELEASE) {
        pressed_keys.erase(key);
	}

	return eventHandled;
}
