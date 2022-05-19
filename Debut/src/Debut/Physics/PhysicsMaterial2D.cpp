#include <Debut/dbtpch.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <yaml-cpp/yaml.h>

namespace Debut
{
	PhysicsMaterial2D::PhysicsMaterial2D(const std::string& path)
	{
		PhysicsMaterial2DConfig config;
		std::stringstream strStream;
		std::ifstream file(path);

		if (file.good())
		{
			strStream << file.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());
			
			config.Density = in["Density"].as<float>();
			config.Friction = in["Friction"].as<float>();
			config.Restitution = in["Restitution"].as<float>();
			config.RestitutionThreshold = in["RestitutionThreshold"].as<float>();
		}
		else
		{
			config = { 1.0f, 0.2f, 0.3f, 0.3f };
		}

		SetConfig(config);
	}

	PhysicsMaterial2D::PhysicsMaterial2D(const PhysicsMaterial2DConfig& config)
	{
		SetConfig(config);
	}

	void PhysicsMaterial2D::SetConfig(const PhysicsMaterial2DConfig& config)
	{
		m_Density = config.Density;
		m_Friction = config.Friction;
		m_Restituion = config.Restitution;
		m_RestitutionThreshold = config.RestitutionThreshold;
	}

	void PhysicsMaterial2D::SaveDefaultConfig(const std::string& path)
	{
		YAML::Emitter out;
		std::ofstream outFile(path);

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PhysicsMaterial2D";
		out << YAML::Key << "Density" << YAML::Value << 1.0f;
		out << YAML::Key << "Friction" << YAML::Value << 0.5f;
		out << YAML::Key << "Restitution" << YAML::Value << 0.5f;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << 0.5f;

		outFile << out.c_str();
	}
}