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
    vec3 original_normal;
} vertex_out;

void main() {
    vertex_out.position = position;
    vertex_out.color = color;

    // Don't normalize yet, we need to do it after fragment shader
    // interpolation anyway.
    vertex_out.normal = NormalMatrix * normal;
    vertex_out.original_normal = normal;
	gl_Position = P * V * M * vec4(position, 1.0);
}
