#version 430

in vertexData
{
    vec3 position;
    vec3 normal;
    float ambient_occlusion;
    vec3 original_normal;
} vertex_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler3D density_map;
layout(binding = 1) uniform sampler2D rock_texture;
layout(binding = 2) uniform sampler2D normal_map;

uniform bool triplanar_colors;

vec3 calculateBlendWeights()
{
    // Doesn't matter whether the normal is pointing along or opposite to the axes.
    vec3 blend_weights = abs(normalize(vertex_in.original_normal));
    // We don't want the blending to happen gradually, only blend in the neighborhood
    // of 45 degree angles.
    blend_weights = blend_weights - 0.45;
    blend_weights = max(blend_weights, 0.0);
    // Make sure weights sum to one.
    blend_weights = blend_weights / (blend_weights.x + blend_weights.y + blend_weights.z);

    return blend_weights;
}

vec3 calculateNormalMap(vec3 texture_normal)
{
    // http://learnopengl.com/#!Advanced-Lighting/Normal-Mapping

    // Need the normal to be in the range [-1, 1];
    texture_normal = normalize(texture_normal * 2 - 1.0);

    return texture_normal;
}

void main() {
    // Need to normalize since interpolation probably changed the lengths.
    vec3 normal = normalize(vertex_in.normal);

    vec3 blend_weights = calculateBlendWeights();

    // TODO: Pass these in as parameters.
    vec3 light_position = vec3(20, 20, 0);
    vec3 light_ambient = vec3(0.3);
    vec3 light_diffuse = vec3(0.3);
    vec3 light_specular = vec3(0.3);
    float shininess = 2;

    // https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
    vec3 L = normalize(light_position - vertex_in.position);
    vec3 E = normalize(-vertex_in.position); // position of eye coordinate is (0, 0, 0) is view space
    vec3 R = normalize(-reflect(L, normal));

    vec3 ambient = light_ambient;

    vec3 diffuse = light_diffuse * max(dot(normal, L), 0.0);
    diffuse = clamp(light_diffuse, 0.0, 1.0);

    vec3 specular = light_specular * pow(max(dot(R, E), 0.0), shininess);
    specular = clamp(specular, 0.0, 1.0);

    if (triplanar_colors) {
        fragColor = vec4(blend_weights, 1.0);
    } else {
        vec4 texture_color =
            texture(rock_texture, vertex_in.position.yz / 32) * blend_weights.x +
            texture(rock_texture, vertex_in.position.xz / 32) * blend_weights.y +
            texture(rock_texture, vertex_in.position.xy / 32) * blend_weights.z;
        fragColor = texture_color * vec4((ambient + diffuse + specular), 1.0);
    }
}
