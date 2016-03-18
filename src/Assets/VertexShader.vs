#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 NormalMatrix;

in vec3 position;
in vec3 color;
in vec3 normal;

out vertexData
{
    vec3 position;
    vec3 color;
    vec3 normal;
} vertex_out;

void main() {
    vertex_out.position = position;
    vertex_out.color = color;
    vertex_out.normal = NormalMatrix * normal;
	gl_Position = P * V * M * vec4(position, 1.0);
}
