#include <Debut/dbtpch.h>
#include <Debut/Physics/PhysicsMaterial3D.h>
#include <yaml-cpp/yaml.h>

namespace Debut
{
	PhysicsMaterial3DConfig PhysicsMaterial3D::DefaultSettings;

	PhysicsMaterial3D::PhysicsMaterial3D(const std::string& path, const std::string& metaFile)
	{
		PhysicsMaterial3DConfig config;
		std::stringstream strStream;
		std::ifstream file(path);

		if (file.good())
		{
			strStream << file.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			config.Friction = in["Friction"].as<float>();
			config.Restitution = in["Restitution"].as<float>();
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
			config = { 0.5f, 0.2f };
		}

		SetConfig(config);
	}

	PhysicsMaterial3D::PhysicsMaterial3D(const std::string& path, const PhysicsMaterial3DConfig& config)
	{
		m_Path = path;
		SetConfig(config);
	}

	void PhysicsMaterial3D::SetConfig(const PhysicsMaterial3DConfig& config)
	{
		m_Friction = config.Friction;
		m_Restitution = config.Restitution;
	}

	void PhysicsMaterial3D::SaveSettings(const std::string& path, const PhysicsMaterial3DConfig& config)
	{
		YAML::Emitter out;

		std::ofstream outFile(path);

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PhysicsMaterial3D";
		out << YAML::Key << "Friction" << YAML::Value << config.Friction;
		out << YAML::Key << "Restitution" << YAML::Value << config.Restitution;
		out << YAML::EndMap << YAML::EndDoc;

		outFile << out.c_str();
	}

	void PhysicsMaterial3D::SaveDefaultConfig(const std::string& path)
	{
		YAML::Emitter out;
		YAML::Emitter outMeta;

		std::ofstream outFile(path, std::ios::out | std::ios::trunc);
		std::ofstream metaFile(path + ".meta");

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PhysicsMaterial3D";
		out << YAML::Key << "Density" << YAML::Value << 1.0f;
		out << YAML::Key << "Friction" << YAML::Value << 0.5f;
		out << YAML::Key << "Restitution" << YAML::Value << 0.5f;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << 0.5f;

		outFile << out.c_str();

		outMeta << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << UUID() << YAML::EndMap;
		metaFile << outMeta.c_str();
	}

	PhysicsMaterial3DConfig PhysicsMaterial3D::GetConfig(const std::string& path)
	{
		PhysicsMaterial3DConfig config;
		std::stringstream strStream;
		std::ifstream file(path);

		if (file.good())
		{
			strStream << file.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			config.Friction = in["Friction"].as<float>();
			config.Restitution = in["Restitution"].as<float>();

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
			config = { 0.5f, 0.2f };

		return config;
	}

	void PhysicsMaterial3D::Reload()
	{
		PhysicsMaterial3DConfig config = GetConfig(m_Path);
		m_Friction = config.Friction;
		m_Restitution = config.Restitution;

		m_ID = config.ID;
	}
}