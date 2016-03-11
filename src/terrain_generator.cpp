#include "terrain_generator.hpp"

using namespace std;

TerrainGenerator::TerrainGenerator()
{
}

void TerrainGenerator::init(string dir)
{
    terrain_shader.generateProgramObject();
    terrain_shader.attachComputeShader((dir + "TerrainDensityShader.cs").c_str());
    terrain_shader.link();
}
