#include <Debut/dbtpch.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <yaml-cpp/yaml.h>

namespace Debut
{
	PhysicsMaterial2DConfig PhysicsMaterial2D::DefaultSettings;

	PhysicsMaterial2D::PhysicsMaterial2D(const std::string& path, const std::string& metaFile)
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
			m_Path = path;

			file.close();

			// Try loading the id as well
			file.open(path + ".meta");
			if (file.good())
			{
				strStream.str("");
				strStream << file.rdbuf();
				in = YAML::Load(strStream.str().c_str());
				m_ID = in["ID"].as<uint64_t>();
			}
		}
		else
		{
			config = { 1.0f, 0.2f, 0.3f, 0.3f };
		}

		SetConfig(config);
	}

	PhysicsMaterial2D::PhysicsMaterial2D(const std::string& path, const PhysicsMaterial2DConfig& config)
	{
		m_Path = path;
		SetConfig(config);
	}

	void PhysicsMaterial2D::SetConfig(const PhysicsMaterial2DConfig& config)
	{
		m_Density = config.Density;
		m_Friction = config.Friction;
		m_Restitution = config.Restitution;
		m_RestitutionThreshold = config.RestitutionThreshold;
	}

	void PhysicsMaterial2D::SaveSettings(const std::string& path, const PhysicsMaterial2DConfig& config)
	{
		YAML::Emitter out;

		std::ofstream outFile(path);

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PhysicsMaterial2D";
		out << YAML::Key << "Density" << YAML::Value << config.Density;
		out << YAML::Key << "Friction" << YAML::Value << config.Friction;
		out << YAML::Key << "Restitution" << YAML::Value << config.Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << config.RestitutionThreshold;
		out << YAML::EndMap << YAML::EndDoc;

		outFile << out.c_str();
	}

	void PhysicsMaterial2D::SaveDefaultConfig(const std::string& path)
	{
		YAML::Emitter out;
		YAML::Emitter outMeta;

		std::ofstream outFile(path, std::ios::out | std::ios::trunc);
		std::ofstream metaFile(path + ".meta");

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PhysicsMaterial2D";
		out << YAML::Key << "Density" << YAML::Value << 1.0f;
		out << YAML::Key << "Friction" << YAML::Value << 0.5f;
		out << YAML::Key << "Restitution" << YAML::Value << 0.5f;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << 0.5f;

		outFile << out.c_str();
		
		outMeta << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << UUID() << YAML::EndMap; 
		metaFile << outMeta.c_str();
	}

	PhysicsMaterial2DConfig PhysicsMaterial2D::GetConfig(const std::string& path)
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

			file.close();

			// Try loading the id as well
			file.open(path + ".meta");
			if (file.good())
			{
				strStream.str("");
				strStream << file.rdbuf();
				in = YAML::Load(strStream.str().c_str());
				config.ID = in["ID"].as<uint64_t>();
			}
		}
		else
			config = { 1.0f, 0.2f, 0.3f, 0.3f, UUID() };

		return config;
	}

	void PhysicsMaterial2D::Reload()
	{
		PhysicsMaterial2DConfig config = GetConfig(m_Path);
		m_Density = config.Density;
		m_Friction = config.Friction;
		m_Restitution = config.Restitution;
		m_RestitutionThreshold = config.RestitutionThreshold;

		m_ID = config.ID;
	}
}