#include "TerrainEditValues.h"
#include "WorldModifier.h"
#include "FastNoiseLite.h"
#include "Utils.h"

namespace TheWorld_Utils
{
	void TerrainEdit::init(enum class TerrainEdit::TerrainType terrainType)
	{
		switch (terrainType)
		{
		case TerrainEdit::TerrainType::campaign_1:
		{
			noise.noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			noise.rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.noiseSeed = 1337;
			noise.frequency = 0.0005f;
			noise.fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			noise.fractalOctaves = 3;
			noise.fractalLacunarity = 2.0f;
			noise.fractalGain = 10.0f;
			noise.fractalWeightedStrength = 0.0f;
			noise.fractalPingPongStrength = 2.0f;

			noise.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			noise.cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			noise.cellularJitter = 1.0f;

			noise.warpNoiseDomainWarpType = -1;
			noise.warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.warpNoiseSeed = 1337;
			noise.warpNoiseDomainWarpAmp = 30.0f;
			noise.warpNoiseFrequency = 0.005f;
			noise.warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			noise.warpNoiseFractalOctaves = 5;
			noise.warpNoiseFractalLacunarity = 2.0f;
			noise.warpNoiseFractalGain = 0.5f;

			amplitude = 100;
		}
		break;
		case TerrainEdit::TerrainType::campaign_2:
		{
			noise.noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			noise.rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.noiseSeed = 1337;
			noise.frequency = 0.00008f;
			noise.fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			noise.fractalOctaves = 3;
			noise.fractalLacunarity = 2.0f;
			noise.fractalGain = 10.0f;
			noise.fractalWeightedStrength = 0.0f;
			noise.fractalPingPongStrength = 2.0f;

			noise.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			noise.cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			noise.cellularJitter = 1.0f;

			noise.warpNoiseDomainWarpType = -1;
			noise.warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.warpNoiseSeed = 1337;
			noise.warpNoiseDomainWarpAmp = 30.0f;
			noise.warpNoiseFrequency = 0.005f;
			noise.warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			noise.warpNoiseFractalOctaves = 5;
			noise.warpNoiseFractalLacunarity = 2.0f;
			noise.warpNoiseFractalGain = 0.5f;

			amplitude = 1000;
		}
		break;
		case TerrainEdit::TerrainType::noise_1:
		{
			noise.noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			noise.rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.noiseSeed = 1337;
			noise.frequency = 0.0005f;
			noise.fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			noise.fractalOctaves = 5;
			noise.fractalLacunarity = 2.0f;
			noise.fractalGain = 0.5f;
			noise.fractalWeightedStrength = 0.0f;
			noise.fractalPingPongStrength = 2.0f;

			noise.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			noise.cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			noise.cellularJitter = 1.0f;

			noise.warpNoiseDomainWarpType = -1;
			noise.warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.warpNoiseSeed = 1337;
			noise.warpNoiseDomainWarpAmp = 30.0f;
			noise.warpNoiseFrequency = 0.005f;
			noise.warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			noise.warpNoiseFractalOctaves = 5;
			noise.warpNoiseFractalLacunarity = 2.0f;
			noise.warpNoiseFractalGain = 0.5f;

			amplitude = 1000;
		}
		break;
		default:
		{
			noise.noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			noise.rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.noiseSeed = 0;
			noise.frequency = 0.0f;
			noise.fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			noise.fractalOctaves = 0;
			noise.fractalLacunarity = 0.0f;
			noise.fractalGain = 0.0f;
			noise.fractalWeightedStrength = 0.0f;
			noise.fractalPingPongStrength = 0.0f;

			noise.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			noise.cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			noise.cellularJitter = 1.0f;

			noise.warpNoiseDomainWarpType = -1;
			noise.warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noise.warpNoiseSeed = 0;
			noise.warpNoiseDomainWarpAmp = 0.0f;
			noise.warpNoiseFrequency = 0.0f;
			noise.warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			noise.warpNoiseFractalOctaves = 0;
			noise.warpNoiseFractalLacunarity = 0.0f;
			noise.warpNoiseFractalGain = 0.0f;

			amplitude = 0;
		}
		break;
		}
	}
}
