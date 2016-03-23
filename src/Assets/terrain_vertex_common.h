vec3 normalAtVertex(vec3 vertex)
{
    float d = 1.0;
    vec3 gradient = vec3(
        density(vertex + vec3(d, 0, 0)) - density(vertex - vec3(d, 0, 0)),
        density(vertex + vec3(0, d, 0)) - density(vertex - vec3(0, d, 0)),
        density(vertex + vec3(0, 0, d)) - density(vertex - vec3(0, 0, d)));
    return -normalize(gradient);
}

float ambientOcclusion(vec3 vertex, bool short_range_ambient, bool long_range_ambient)
{
    // TODO: Need to make sure ambient occlusion looks the same for all block sizes.
    float visibility = 0.0;
    for (int i = 0; i < 32; i++) {
        vec3 ray = random_rays[i];
        float ray_visibility = 1.0;

        // Short-range samples
        // Don't use multiplication! Adding is faster.
        // Start some (large) epsilon away.
        if (short_range_ambient) {
            vec3 short_ray = vertex + ray * 0.1;
            vec3 delta = ray / 4;
            for (int j = 0; j < 16; j++) {
                short_ray += delta;
                float d = density(short_ray);
                ray_visibility *= clamp(d * 8, 0.0, 1.0);
            }
        }

        // Long-range samples
        if (long_range_ambient) {
            for (int j = 0; j < 4; j++) {
                float distance = pow((j + 2) / 5.0, 1.8) * 40;
                float d = terrainDensity(vertex + distance * ray, block_size, period, 3);
                ray_visibility *= clamp(d * 0.5, 0.0, 1.0);
            }
        }

        visibility += ray_visibility;
    }

    return (1.0 - visibility / 32.0);
}
