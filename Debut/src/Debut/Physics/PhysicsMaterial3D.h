#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>

namespace Debut
{
	struct PhysicsMaterial3DConfig
	{
		float Friction;
		float Restitution;
		UUID ID;
	};

	class PhysicsMaterial3D
	{
		friend class PropertiesPanel;
	public:
		static PhysicsMaterial3DConfig DefaultSettings;

		PhysicsMaterial3D() = default;
		PhysicsMaterial3D(const std::string & path, const std::string & metaFile = "");
		PhysicsMaterial3D(const std::string & path, const PhysicsMaterial3DConfig & config);

		static void SaveDefaultConfig(const std::string & path);
		static void SaveSettings(const std::string & path, const PhysicsMaterial3DConfig & config);
		static PhysicsMaterial3DConfig GetConfig(const std::string & path);

		void Reload();

		std::string GetPath() { return m_Path; }
		float GetFriction() { return m_Friction; }
		float GetRestitution() { return m_Restitution; }
		UUID GetID() { return m_ID; }

		void SetConfig(const PhysicsMaterial3DConfig & config);

	private:
		std::string m_Path;
		UUID m_ID;

		float m_Friction;
		float m_Restitution;
	};
}