#version 430

uniform float period;

// One work group = 1 slice
// Note that 32x32 = 1024 which is the typical maximum work group size/block
// size for GPGPU languages, including CUDA, etc.
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// One float per texture location.
layout(r32f, binding = 0) uniform image3D density_map;

// Represent the twelve vectors of the edges of a cube.
// Note that since these are not normalize, the range of the perlin
// noise will be in [-2, 2]
vec3 perlinVectors[12] = {
    vec3(1,1,0),vec3(-1,1,0),vec3(1,-1,0),vec3(-1,-1,0),
    vec3(1,0,1),vec3(-1,0,1),vec3(1,0,-1),vec3(-1,0,-1),
    vec3(0,1,1),vec3(0,-1,1),vec3(0,1,-1),vec3(0,-1,-1)
};

vec3 gradientAtCoordinate(ivec3 gridCoords)
{
    int hash = gridCoords.x * 191 + gridCoords.y * 389 + gridCoords.z * 431;
    return perlinVectors[hash % 12];
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

float perlinNoise(ivec3 coords, float frequency)
{
    vec3 scaledCoords = vec3(coords) * frequency;
    vec3 innerCoords = vec3(mod(scaledCoords, 1.0));
    ivec3 lowerCorner = ivec3(scaledCoords - innerCoords);

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
    vec2 yInterp = mix(zInterp.xy, zInterp.zw, yInterpolant);
    float xInterp = mix(yInterp.x, yInterp.y, xInterpolant);
    return xInterp;
}

void main() {
    ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 block_dimensions = ivec3(gl_NumWorkGroups * gl_WorkGroupSize);

    // Air is negative, ground is positive.
    // Generate a gradient from [1 to -1]
    float height_gradient = 1.0 - (float(coords.y) / block_dimensions.y) * 2;

    float noise = 0.0;
    float frequency = 1.0 / period;
    noise += perlinNoise(coords, frequency);
    frequency *= 1.95;
    noise += perlinNoise(coords, frequency) / 2;
    frequency *= 1.95;
    noise += perlinNoise(coords, frequency) / 3;
    frequency *= 1.95;
    noise += perlinNoise(coords, frequency) / 4;
    frequency *= 1.95;
    noise += perlinNoise(coords, frequency) / 5;
    frequency *= 1.95;
    noise += perlinNoise(coords, frequency) / 6;

    float density = height_gradient + noise * 0.5;

    //imageStore(density_map, coords, mod(vec4(frequency), 1.0));
    //imageStore(density_map, coords, vec4(perlinNoise(coords, frequency)));
    imageStore(density_map, coords, vec4(density));
}
