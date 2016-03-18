#version 430

in vertexData
{
    vec3 position;
    vec3 color;
    vec3 normal;
    vec3 original_normal;
} vertex_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler3D density_map;

void main() {
    // Need to normalize since interpolation probably changed the lengths.
    vec3 normal = normalize(vertex_in.normal);

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

    // Doesn't matter whether the normal is pointing along or opposite to the axes.
    vec3 blend_weights = abs(normalize(vertex_in.original_normal));
    // We don't want the blending to happen gradually, only blend in the neighborhood
    // of 45 degree angles.
    blend_weights = blend_weights - 0.4;
    blend_weights = max(blend_weights, 0.0);
    // Make sure weights sum to one.
    blend_weights = blend_weights / (blend_weights.x + blend_weights.y + blend_weights.z);

    vec3 color;
    if (true) {
        color = blend_weights;
    } else {
        // TODO: texture blending
    }

    fragColor = vec4(color, 1);
    //fragColor = vec4(vertex_in.color * (ambient + diffuse + specular), 1);
}
