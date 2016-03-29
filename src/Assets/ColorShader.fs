#version 430

uniform vec4 color;

uniform vec3 eye_position;
uniform vec3 fog_params;

in vec3 world_position;

out vec4 fragColor;

void main() {
    float vertex_distance = length(eye_position - world_position);
    float fog_falloff = clamp(fog_params.x * vertex_distance / fog_params.y - fog_params.z, 0.0, 1.0);
    fragColor = vec4(mix(color.rgb, vec3(0.5, 0.5, 0.5), fog_falloff), color.a);
}
