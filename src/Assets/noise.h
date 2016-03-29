// Represent the twelve vectors of the edges of a cube.
// Note that since these are not normalize, the range of the perlin
// noise will be in [-2, 2]
vec3 perlinVectors[12] = {
    vec3(1,1,0),vec3(-1,1,0),vec3(1,-1,0),vec3(-1,-1,0),
    vec3(1,0,1),vec3(-1,0,1),vec3(1,0,-1),vec3(-1,0,-1),
    vec3(0,1,1),vec3(0,-1,1),vec3(0,1,-1),vec3(0,-1,-1)
};

unsigned int hash(ivec3 lowerCorner) {
    unsigned int x = lowerCorner.x * 256 * 256 + lowerCorner.y * 256 + lowerCorner.z;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

vec3 gradientAtCoordinate(ivec3 gridCoords)
{
    return perlinVectors[hash(gridCoords) % 12];
}

float influenceAtCoordinate(ivec3 lowerCorner, ivec3 offset, vec3 innerCoords)
{
    return dot(gradientAtCoordinate(lowerCorner + offset), innerCoords - offset);
}

// Perlin interpolant easing function that has first and second derivatives
// equal to zero at the endpoints.
float ease(float t)
{
    float t3 = t * t * t;
    float t4 = t3 * t;
    float t5 = t4 * t;
    return 6 * t5 - 15 * t4 + 10 * t3;
}

float perlinNoise(vec3 coords, float frequency)
{
    vec3 scaledCoords = vec3(coords) * frequency;
    vec3 innerCoords = vec3(mod(scaledCoords, 1.0));

    // Need to use floor first to truncate consistently towards negative
    // infinity. Otherwise, there will be symmetry around 0.
    ivec3 lowerCorner = ivec3(floor(scaledCoords));

    // For swizzling.
    ivec2 offset = ivec2(0, 1);

    float xInterpolant = ease(innerCoords.x);
    float yInterpolant = ease(innerCoords.y);
    float zInterpolant = ease(innerCoords.z);

    // Calculate and store the influence at each corner from the gradients.
    vec4 face1 = vec4(influenceAtCoordinate(lowerCorner, offset.xxx, innerCoords),
                      influenceAtCoordinate(lowerCorner, offset.xyx, innerCoords),
                      influenceAtCoordinate(lowerCorner, offset.yxx, innerCoords),
                      influenceAtCoordinate(lowerCorner, offset.yyx, innerCoords));
    vec4 face2 = vec4(influenceAtCoordinate(lowerCorner, offset.xxy, innerCoords),
                      influenceAtCoordinate(lowerCorner, offset.xyy, innerCoords),
                      influenceAtCoordinate(lowerCorner, offset.yxy, innerCoords),
                      influenceAtCoordinate(lowerCorner, offset.yyy, innerCoords));
    vec4 zInterp = mix(face1, face2, zInterpolant);
    vec2 yInterp = mix(zInterp.xz, zInterp.yw, yInterpolant);
    float xInterp = mix(yInterp.x, yInterp.y, xInterpolant);
    return xInterp;
}

// coords should be in the range [coords, block_size] not [0, 1]
float terrainDensity(vec3 coords, float block_size, float period, int octaves, float octaves_decay)
{
    float max_blocks_y = 2.0;

    // Air is negative, ground is positive.
    // Generate a gradient from [max to min]
    float min = -1.0;
    float max = 0.5;
    float height_gradient = max - (max - min) * (coords.y / max_blocks_y) / block_size;

    float noise = 0.0;
    float frequency = 1.0 / period;

    for (int i = 1; i <= octaves; i++) {
        noise += perlinNoise(coords, frequency) / pow(i, octaves_decay);
        frequency *= 1.95;
    }

    float density = height_gradient + noise * 1.5;

    // Should make sure that there's a solid ground at the bottom
    // and air at the top.
    if (coords.y / block_size < 0.1) {
        density += (0.1 - coords.y / block_size) * 10;
    }
    if (coords.y / block_size > max_blocks_y - 0.1) {
        density -= (coords.y / block_size - (max_blocks_y - 0.1)) * 10;
    }

    // For debugging : this is the density function for a sphere.
    //density = length(coords - ivec3(32, 32, 32)) - 16 + period * 0.0001;

    return density;
}
