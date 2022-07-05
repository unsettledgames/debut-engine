#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Model.h>
#include <yaml-cpp/yaml.h>

namespace Debut
{
	Model::Model(const std::string& path, const std::string& metaFile)
	{
		std::string correctMeta = metaFile;
		if (correctMeta == "")
			correctMeta = path + ".meta";

		// Check meta etc etc
		std::ifstream meta(correctMeta);
		if (meta.good())
		{
			std::stringstream ss;
			ss << meta.rdbuf();
			YAML::Node node = YAML::Load(ss.str());

			m_ID = node["ID"].as<uint64_t>();
			m_SubModels.resize(node["NumSubModels"].as<uint32_t>());
			m_Meshes.resize(node["NumMeshes"].as<uint32_t>());
			m_Materials.resize(node["NumMaterials"].as<uint32_t>());

			meta.close();
			ss.str("");
			std::ifstream inFile(path);
			ss << inFile.rdbuf();
			node = YAML::Load(ss.str());

			for (uint32_t i = 0; i < m_SubModels.size(); i++)
				m_SubModels[i] = node["SubModels"][i].as<uint64_t>();
			for (uint32_t i = 0; i < m_Materials.size(); i++)
				m_Materials[i] = node["Materials"][i].as<uint64_t>();
			for (uint32_t i = 0; i < m_Meshes.size(); i++)
				m_Meshes[i] = node["Meshes"][i].as<uint64_t>();

			m_Valid = true;
		}
		else
		{
			m_Valid = false;
		}
	}

	void Model::SaveSettings()
	{
		YAML::Emitter emitter;
		std::ofstream out(m_Path);

		emitter << YAML::BeginDoc << YAML::BeginMap;
		emitter << YAML::Key << "SubModels" << YAML::Value << YAML::BeginSeq;

		// Submodels
		for (uint32_t i = 0; i < m_SubModels.size(); i++)
			emitter << m_SubModels[i];
		emitter << YAML::EndSeq;

		// Materials
		emitter << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < m_Materials.size(); i++)
			emitter << m_Materials[i];
		emitter << YAML::EndSeq;

		// Meshes
		emitter << YAML::Key << "Meshes" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < m_Meshes.size(); i++)
			emitter << m_Meshes[i];
		emitter << YAML::EndSeq;

		emitter << YAML::EndMap << YAML::EndDoc;
		out << emitter.c_str();

		// Save meta
		out.close();
		YAML::Emitter metaEmitter; 
		out.open(m_Path + ".meta");
		
		metaEmitter << YAML::BeginDoc << YAML::BeginMap;
		
		metaEmitter << YAML::Key << "ID" << YAML::Value << m_ID;
		metaEmitter << YAML::Key << "NumSubModels" << YAML::Value << m_SubModels.size();
		metaEmitter << YAML::Key << "NumMeshes" << YAML::Value << m_Meshes.size();
		metaEmitter << YAML::Key << "NumMaterials" << YAML::Value << m_Materials.size();
		
		metaEmitter << YAML::EndMap << YAML::EndDoc;
		out << metaEmitter.c_str();
	}
}