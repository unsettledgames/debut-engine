#include <Debut/dbtpch.h>
#include <Debut/Scripting/ScriptMetadata.h>
#include <yaml-cpp/yaml.h>

namespace Debut
{
	ScriptMetadata::ScriptMetadata(const std::string& filePath, const std::string& metaPath)
	{
		std::string trueMetaPath = metaPath;
		if (trueMetaPath == "")
			trueMetaPath = filePath + ".meta";
		// Load script
		std::ifstream metaFile(trueMetaPath);
		if (!metaFile.good())
		{
			metaFile.close();
			std::ofstream newMeta;
			newMeta.open(filePath + ".meta");

			YAML::Emitter emitter;
			emitter << YAML::BeginDoc << YAML::BeginMap;
			{
				emitter << YAML::Key << "Name" << YAML::Value << filePath.substr(filePath.find_last_of("/") + 1);
				emitter << YAML::Key << "ID" << YAML::Value << m_Script;
			}
			YAML::EndMap << YAML::EndDoc;

			newMeta << emitter.c_str();
		}
		else
		{
			std::stringstream ss;
			ss << metaFile.rdbuf();
			YAML::Node node = YAML::Load(ss.str());

			m_Script = node["ID"].as<uint64_t>();
			m_ClassName = node["Name"].as<std::string>();
		}

		auto sas = filePath.find_last_of("\\");
		m_ClassName = filePath.substr(filePath.find_last_of("\\")+1);
	}
}