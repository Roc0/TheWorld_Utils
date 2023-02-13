#include "TerrainEditValues.h"
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
			noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noiseSeed = 1337;
			frequency = 0.0005f;
			fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			fractalOctaves = 3;
			fractalLacunarity = 2.0f;
			fractalGain = 10.0f;
			fractalWeightedStrength = 0.0f;
			fractalPingPongStrength = 2.0f;

			cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			cellularJitter = 1.0f;

			warpNoiseDomainWarpType = -1;
			warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			warpNoiseSeed = 1337;
			warpNoiseDomainWarpAmp = 30.0f;
			warpNoiseFrequency = 0.005f;
			warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			warpNoiseFractalOctaves = 5;
			warpNoiseFractalLacunarity = 2.0f;
			warpNoiseFractalGain = 0.5f;

			amplitude = 100;
		}
		break;
		case TerrainEdit::TerrainType::campaign_2:
		{
			noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noiseSeed = 1337;
			frequency = 0.00008f;
			fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			fractalOctaves = 3;
			fractalLacunarity = 2.0f;
			fractalGain = 10.0f;
			fractalWeightedStrength = 0.0f;
			fractalPingPongStrength = 2.0f;

			cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			cellularJitter = 1.0f;

			warpNoiseDomainWarpType = -1;
			warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			warpNoiseSeed = 1337;
			warpNoiseDomainWarpAmp = 30.0f;
			warpNoiseFrequency = 0.005f;
			warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			warpNoiseFractalOctaves = 5;
			warpNoiseFractalLacunarity = 2.0f;
			warpNoiseFractalGain = 0.5f;

			amplitude = 1000;
		}
		break;
		case TerrainEdit::TerrainType::noise_1:
		{
			noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noiseSeed = 1337;
			frequency = 0.0005f;
			fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			fractalOctaves = 5;
			fractalLacunarity = 2.0f;
			fractalGain = 0.5f;
			fractalWeightedStrength = 0.0f;
			fractalPingPongStrength = 2.0f;

			cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			cellularJitter = 1.0f;

			warpNoiseDomainWarpType = -1;
			warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			warpNoiseSeed = 1337;
			warpNoiseDomainWarpAmp = 30.0f;
			warpNoiseFrequency = 0.005f;
			warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			warpNoiseFractalOctaves = 5;
			warpNoiseFractalLacunarity = 2.0f;
			warpNoiseFractalGain = 0.5f;

			amplitude = 1000;
		}
		break;
		default:
		{
			noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin;
			rotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			noiseSeed = 0;
			frequency = 0.0f;
			fractalType = FastNoiseLite::FractalType::FractalType_FBm;
			fractalOctaves = 0;
			fractalLacunarity = 0.0f;
			fractalGain = 0.0f;
			fractalWeightedStrength = 0.0f;
			fractalPingPongStrength = 0.0f;

			cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
			cellularReturnType = FastNoiseLite::CellularReturnType::CellularReturnType_Distance;
			cellularJitter = 1.0f;

			warpNoiseDomainWarpType = -1;
			warpNoiseRotationType3D = FastNoiseLite::RotationType3D::RotationType3D_None;
			warpNoiseSeed = 0;
			warpNoiseDomainWarpAmp = 0.0f;
			warpNoiseFrequency = 0.0f;
			warpNoieseFractalType = FastNoiseLite::FractalType::FractalType_None;
			warpNoiseFractalOctaves = 0;
			warpNoiseFractalLacunarity = 0.0f;
			warpNoiseFractalGain = 0.0f;

			amplitude = 0;
		}
		break;
		}
	}
}
