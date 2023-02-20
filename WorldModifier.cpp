#include "pch.h"

#define _USE_MATH_DEFINES // for C++

#include "math.h"
#include "assert.h"
#include "WorldModifier.h"

namespace TheWorld_Utils
{
	WorldModifier::WorldModifier()
	{
		//m_strength = 0.0;
		m_AOE = 0.0;
		m_radius = 0.0;
		m_azimuth = 0.0;
		m_azimuthDegree = 0.0;
		m_functionType = WMFunctionType::none;
		m_rowid = -1;
		m_order = WMOrder::MaxEffectOnWM;
		m_min = 0.0f;
		m_max = 0.0f;
	}

	WorldModifier::WorldModifier(WorldModifierPos& pos, enum class WMFunctionType functionType, float AOE, float min, float max, enum class WMOrder order, void* fp) : m_pos(pos)
	{
		//if (strength < 0.0 || strength > 1.0)
		//	throw(MapManagerExceptionWrongInput("Strength of a WM must be in range 0.0 / 1.0"));
		
		m_functionType = functionType;
		//m_strength = strength;
		m_AOE = AOE;
		m_order = order;
		m_min = min;
		m_max = max;
		assert(m_min <= m_max);

		m_rowid = -1;

		m_radius = sqrtf(powf(m_pos.getPosX(), 2.0) + powf(m_pos.getPosZ(), 2.0));
		if ((m_pos.getPosX() == 0 && m_pos.getPosZ() == 0) || m_radius == 0)
		{
			m_radius = 0;
			m_azimuth = 0;
			m_azimuthDegree = 0;
		}
		else
		{
			//angle of radius with x-axis (complementar of 2PI if Z < 0)
			m_azimuth = acosf(m_pos.getPosX() / m_radius);
			if (m_pos.getPosZ() < 0)
				m_azimuth = float(M_PI) * (float)2.0 - m_azimuth;
			m_azimuthDegree = (m_azimuth * 180) / float(M_PI);
		}
	}

	void WorldModifier::setInternalValues(WorldModifierPos& pos, float radius, float azimuth, float azimuthDegree, float AOE, enum class WMFunctionType functionType, __int64 rowid, float min, float max, enum class WMOrder order, void* fp)
	{
		m_pos = pos;
		//m_strength = strength;
		m_AOE = AOE;
		m_radius = radius;
		m_azimuth = azimuth;
		m_azimuthDegree = azimuthDegree;
		m_functionType = functionType;

		m_rowid = rowid;

		m_order = order;
		m_min = min;
		m_max = max;
	}
	
	WorldModifier::~WorldModifier()
	{
	}
}