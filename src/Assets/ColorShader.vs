#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec3 position;

out vec3 world_position;

void main() {
    world_position = vec3(M * vec4(position, 1.0));
	gl_Position = P * V * vec4(world_position, 1.0);
}
