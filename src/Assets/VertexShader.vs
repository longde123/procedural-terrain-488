#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 NormalMatrix;

in vec3 position;
in vec3 normal;
in float ambient_occlusion;

out vertexData
{
    vec3 position;
    vec3 normal;
    float ambient_occlusion;
} vertex_out;

void main() {
    vertex_out.position = position;
    vertex_out.ambient_occlusion = ambient_occlusion;

    // Don't normalize yet, we need to do it after fragment shader
    // interpolation anyway.
    vertex_out.normal = NormalMatrix * normal;

	gl_Position = P * V * M * vec4(position, 1.0);
}
