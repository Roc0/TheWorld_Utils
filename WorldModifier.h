#pragma once

namespace TheWorld_Utils
{
	class WorldModifier;
	
	enum class WMType
	{
		none = -1
		,elevator = 0
		//,depressor = 1
		//,flattener = 2
	};

	class WorldModifierPos
	{
		friend WorldModifier;
	public:
		_declspec(dllexport) WorldModifierPos()
		{
			m_level = 0;
			m_posX = 0.0f;
			m_posZ = 0.0f;
			m_type = WMType::none;
			m_priority = 0;
		}

		_declspec(dllexport) WorldModifierPos(int level, float posX, float posZ, enum class WMType type, int priority)
		{
			m_level = level;
			m_posX = posX;
			m_posZ = posZ;
			m_type = type;
			m_priority = priority;
		}

		_declspec(dllexport) WorldModifierPos(const WorldModifierPos& wmp)
		{
			*this = wmp;
		}

		void operator=(const WorldModifierPos& wmp)
		{
			m_level = wmp.m_level;
			m_posX = wmp.m_posX;
			m_posZ = wmp.m_posZ;
			m_type = wmp.m_type;
			m_priority = wmp.m_priority;
		}

		// needed to use an istance of WorldModifierPos as a key in a map (to keep the map sorted by m_posZ and by m_posX for equal m_posZ)
		bool operator<(const WorldModifierPos& p) const
		{
			if (m_level < p.m_level)
				return true;
			else if (m_level > p.m_level)
				return false;
			else
			{
				if (m_posZ < p.m_posZ)
					return true;
				else if (m_posZ > p.m_posZ)
					return false;
				else
				{
					if (m_posX < p.m_posX)
						return true;
					else if (m_posX > p.m_posX)
						return false;
					else
					{
						if (m_priority < p.m_priority)
							return true;
						else if (m_priority > p.m_priority)
							return false;
						else
							return m_type < p.m_type;
					}
				}
			}
		}

		_declspec(dllexport) int getLevel(void) const
		{
			return m_level;
		};
		_declspec(dllexport) float getPosX(void) const
		{
			return m_posX;
		};
		_declspec(dllexport) float getPosZ(void) const
		{
			return m_posZ;
		};
		_declspec(dllexport) enum class WMType getType(void) const
		{
			return m_type;
		};
		_declspec(dllexport) int getPriority(void) const
		{
			return m_priority;
		};

	private:
		int m_level;
		float m_posX;
		float m_posZ;
		enum class WMType m_type;
		int m_priority;
	};

	enum class WMFunctionType
	{
		none = -1
		,Add = 0
		,Mulitply = 1
	};

	enum class WMOrder
	{
		none = -1
		,MaxEffectOnWM = 0
		,MinEffectOnWM = 1
	};

	class WorldModifier
	{
	public:
		_declspec(dllexport) WorldModifier();
		WorldModifier(const WorldModifier& wm)
		{
			*this = wm;
		}

		_declspec(dllexport) WorldModifier(WorldModifierPos& pos, enum class WMFunctionType functionType, float AOE, float min, float max, enum class WMOrder order, void* fp = nullptr);
		_declspec(dllexport) ~WorldModifier();

		_declspec(dllexport) void setInternalValues(WorldModifierPos& pos, float radius, float azimuth, float azimuthDegree, float AOE, enum class WMFunctionType functionType, __int64 rowid, float min, float max, enum class WMOrder order, void* fp = nullptr);

		_declspec(dllexport) WorldModifierPos getPos(void) const
		{
			return m_pos;
		};

		//_declspec(dllexport) float getStrength(void) const
		//{
		//	return m_strength; 
		//};
		_declspec(dllexport) float getAOE(void) const
		{
			return m_AOE; 
		};
		_declspec(dllexport) float getRadius(void) const
		{
			return m_radius; 
		};
		_declspec(dllexport) float getAzimuth(void) const
		{
			return m_azimuth; 
		};
		_declspec(dllexport) float getAzimuthDegree(void) const
		{
			return m_azimuthDegree; 
		};
		_declspec(dllexport) enum class WMFunctionType getFunctionType(void) const
		{
			return m_functionType; 
		};

		_declspec(dllexport) __int64 getRowid(void) const
		{
			return m_rowid; 
		};

		_declspec(dllexport) enum class WMOrder getOrder(void) const
		{
			return m_order;
		};

		_declspec(dllexport) float getMin(void) const
		{
			return m_min;
		};

		_declspec(dllexport) float getMax(void) const
		{
			return m_max;
		};

		void operator=(const WorldModifier& wm)
		{
			m_pos = wm.m_pos;
			//m_strength = wm.m_strength;
			m_AOE = wm.m_AOE;
			m_radius = wm.m_radius;
			m_azimuth = wm.m_azimuth;
			m_azimuthDegree = wm.m_azimuthDegree;
			m_functionType = wm.m_functionType;
			m_rowid = wm.m_rowid;
			m_order = wm.m_order;
			m_min = wm.m_min;
			m_max = wm.m_max;
		}

	private:
		WorldModifierPos m_pos;
		//float m_strength;
		float m_AOE;
		float m_radius;
		float m_azimuth;
		float m_azimuthDegree;
		enum class WMFunctionType m_functionType;
		__int64 m_rowid;
		enum class WMOrder m_order;
		float m_min;
		float m_max;
	};
}


