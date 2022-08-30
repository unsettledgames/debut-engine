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
		UUID ID;
	};

	class PhysicsMaterial2D
	{
		friend class PropertiesPanel;

	public:
		
		static PhysicsMaterial2DConfig DefaultSettings;

		PhysicsMaterial2D() = default;
		PhysicsMaterial2D(const std::string& path, const std::string& metaFile = "");
		PhysicsMaterial2D(const std::string& path, const PhysicsMaterial2DConfig& config);

		static void SaveDefaultConfig(const std::string& path);
		static void SaveSettings(const std::string& path, const PhysicsMaterial2DConfig& config);
		static PhysicsMaterial2DConfig GetConfig(const std::string& path);

		void Reload();

		std::string GetPath() { return m_Path; }
		float GetDensity() { return m_Density; }
		float GetFriction() { return m_Friction; }
		float GetRestitution() { return m_Restitution; }
		float GetRestitutionThreshold() { return m_RestitutionThreshold; }
		UUID GetID() { return m_ID; }

		void SetConfig(const PhysicsMaterial2DConfig& config);		
		
	private:
		std::string m_Path;
		UUID m_ID;

		float m_Density;
		float m_Friction;
		float m_Restitution;
		float m_RestitutionThreshold;
	};
}