#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec3 position;

out vertexData
{
    vec3 position;
} vertex_out;

void main() {
    vertex_out.position = position;
    gl_Position = P * V * M * vec4(position, 1.0);
}
