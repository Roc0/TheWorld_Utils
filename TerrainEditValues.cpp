#include "TerrainEditValues.h"
#include "WorldModifier.h"
#include "FastNoiseLite.h"
#include "Utils.h"

namespace TheWorld_Utils
{
	std::string TerrainEdit::getDefaultTextureNameForTerrainType(enum class TerrainEdit::TerrainType terrainType, enum class TerrainEdit::TextureType textureType)
	{
		std::string ret = "";

		switch (terrainType)
		{
		case TerrainType::campaign_1:
			if (textureType == TerrainEdit::TextureType::lowElevation)
				ret = "sand";
				//ret = "Rock028_1K";
			else if (textureType == TerrainEdit::TextureType::highElevation)
				ret = "grass";
			else if (textureType == TerrainEdit::TextureType::dirt)
				ret = "leaves";
			else if (textureType == TerrainEdit::TextureType::rocks)
				ret = "rocks07";
			break;
		case TerrainType::high_mountains_1:
		case TerrainType::high_mountains_1_grow:
		case TerrainType::high_mountains_2:
		case TerrainType::high_mountains_2_grow:
			if (textureType == TerrainEdit::TextureType::lowElevation)
				ret = "Ground003_2K";
			else if (textureType == TerrainEdit::TextureType::highElevation)
				ret = "PaintedPlaster017_1K";
			else if (textureType == TerrainEdit::TextureType::dirt)
				ret = "Ground049C_1K";
			else if (textureType == TerrainEdit::TextureType::rocks)
				ret = "rocks07";
			break;
		default:
			if (textureType == TerrainEdit::TextureType::lowElevation)
				ret = "Ground003_2K";
			else if (textureType == TerrainEdit::TextureType::highElevation)
				ret = "Ground003_2K";
			else if (textureType == TerrainEdit::TextureType::dirt)
				ret = "leaves";
			else if (textureType == TerrainEdit::TextureType::rocks)
				ret = "rocks07";
			break;
		}

		return ret;
	}

	void TerrainEdit::setTextureNameForTerrainType(enum class TerrainEdit::TextureType textureType)
	{
		std::string texName = getDefaultTextureNameForTerrainType(terrainType, textureType);
		if (textureType == TerrainEdit::TextureType::lowElevation)
			strcpy_s(extraValues.lowElevationTexName_r, sizeof(extraValues.lowElevationTexName_r), texName.c_str());
		else if (textureType == TerrainEdit::TextureType::highElevation)
			strcpy_s(extraValues.highElevationTexName_g, sizeof(extraValues.highElevationTexName_g), texName.c_str());
		else if (textureType == TerrainEdit::TextureType::dirt)
			strcpy_s(extraValues.dirtTexName_b, sizeof(extraValues.dirtTexName_b), texName.c_str());
		else if (textureType == TerrainEdit::TextureType::rocks)
			strcpy_s(extraValues.rocksTexName_a, sizeof(extraValues.rocksTexName_a), texName.c_str());
	}

	void TerrainEdit::init(enum class TerrainEdit::TerrainType terrainType)
	{
		std::string s = getDefaultTextureNameForTerrainType(terrainType, TerrainEdit::TextureType::lowElevation);
		strcpy_s(extraValues.lowElevationTexName_r, sizeof(extraValues.lowElevationTexName_r), s.c_str());
		s = getDefaultTextureNameForTerrainType(terrainType, TerrainEdit::TextureType::highElevation);
		strcpy_s(extraValues.highElevationTexName_g, sizeof(extraValues.highElevationTexName_g), s.c_str());
		s = getDefaultTextureNameForTerrainType(terrainType, TerrainEdit::TextureType::dirt);
		strcpy_s(extraValues.dirtTexName_b, sizeof(extraValues.dirtTexName_b), s.c_str());
		s = getDefaultTextureNameForTerrainType(terrainType, TerrainEdit::TextureType::rocks);
		strcpy_s(extraValues.rocksTexName_a, sizeof(extraValues.rocksTexName_a), s.c_str());

		switch (terrainType)
		{
		case TerrainEdit::TerrainType::jagged_1:
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

			noise.amplitude = 100;
			noise.scaleFactor = 1.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = false;
		}
		break;
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

			noise.amplitude = 100;
			noise.scaleFactor = 1.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = true;
		}
		break;
		case TerrainEdit::TerrainType::plateau_1:
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

			noise.amplitude = 100;
			noise.scaleFactor = 1.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = false;
		}
		break;
		case TerrainEdit::TerrainType::low_hills:
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

			noise.amplitude = 1000;
			noise.scaleFactor = 1.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = true;
		}
		break;
		case TerrainEdit::TerrainType::high_hills:
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

			noise.amplitude = 2000;
			noise.scaleFactor = 1.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = true;
		}
		break;
		case TerrainEdit::TerrainType::low_mountains:
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

			noise.amplitude = 2500;
			noise.scaleFactor = 1.0f / 1.5f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = true;
		}
		break;
		case TerrainEdit::TerrainType::low_mountains_grow:
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

			noise.amplitude = 2500;
			noise.scaleFactor = 1.0f / 1.5f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = false;
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_1:
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

			noise.amplitude = 4000;
			noise.scaleFactor = 1.0f / 1.5f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = true;
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_1_grow:
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

			noise.amplitude = 4000;
			noise.scaleFactor = 1.0f / 1.5f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = false;
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_2:
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

			noise.amplitude = 9000;
			noise.scaleFactor = 1.0f / 3.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = true;
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_2_grow:
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

			noise.amplitude = 9000;
			noise.scaleFactor = 1.0f / 3.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = false;
		}
		break;
		case TerrainEdit::TerrainType::noise_1:
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

			noise.amplitude = 0;
			noise.scaleFactor = 1.0f;
			noise.desideredMinHeight = 0.0f;
			noise.desideredMinHeigthMandatory = false;
		}
		break;
		}
	}
}
