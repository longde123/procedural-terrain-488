#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
in vec3 position;

void main() {
	gl_Position = vec4(position.x, gl_InstanceID, position.z, 1.0);
	//gl_Position = P * V * M * vec4(position, 1.0);
}
