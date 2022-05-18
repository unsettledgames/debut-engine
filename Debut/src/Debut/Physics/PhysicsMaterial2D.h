#pragma once

#include <Debut/Core/UUID.h>

namespace Debut
{
	struct PhysicsMaterial2DConfig
	{
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.5f;
		float RestitutionThreshold = 0.5f;
	};

	class PhysicsMaterial2D
	{
	public:
		PhysicsMaterial2D() = default;
		PhysicsMaterial2D(const std::string& config);
		PhysicsMaterial2D(const PhysicsMaterial2DConfig& config);

		static void SaveDefaultConfig(const std::string& path);

		std::string GetPath() { return m_Path; }
		float GetDensity() { return m_Density; }
		float GetFriction() { return m_Friction; }
		float GetRestitution() { return m_Restituion; }
		float GetRestitutionThreshold() { return m_RestitutionThreshold; }

		void SetConfig(const PhysicsMaterial2DConfig& config);		
		
	private:
		std::string m_Path;
		UUID m_ID;

		float m_Density;
		float m_Friction;
		float m_Restituion;
		float m_RestitutionThreshold;
	};
}