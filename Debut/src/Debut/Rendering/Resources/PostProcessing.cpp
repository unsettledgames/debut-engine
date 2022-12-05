#include <Debut/dbtpch.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Resources/PostProcessing.h>
#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>

namespace Debut
{
	PostProcessingStack::PostProcessingStack(const std::string& path, const std::string& metaFile)
	{
		PostProcessingStackConfig config = GetConfig(path);
		SetConfig(config);
	}

	PostProcessingStack::PostProcessingStack(const std::string& path, const PostProcessingStackConfig& config)
	{
		m_Path = path;
		SetConfig(config);
	}

	void PostProcessingStack::SaveDefaultConfig(const std::string& path)
	{
		YAML::Emitter out;
		YAML::Emitter outMeta;

		std::ofstream outFile(path, std::ios::out | std::ios::trunc);
		std::ofstream metaFile(path + ".meta");

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PostProcessingStack";
		out << YAML::Key << "Volumes" << YAML::Value << YAML::BeginSeq << YAML::EndSeq;
		out << YAML::EndMap << YAML::EndDoc;

		outFile << out.c_str();

		outMeta << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << UUID() << YAML::EndMap;
		metaFile << outMeta.c_str();
	}

	void PostProcessingStack::SaveSettings(const std::string& path, const PostProcessingStackConfig& config)
	{
		YAML::Emitter out;

		std::ofstream outFile(path);

		out << YAML::BeginDoc << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << "PostProcessingStack";
		out << YAML::Key << "Volumes" << YAML::Value << YAML::BeginSeq;

		for (auto& volume : config.Volumes)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Name" << YAML::Value << volume.Name;
			out << YAML::Key << "Shader" << YAML::Value << volume.ShaderID;
			out << YAML::Key << "Enabled" << YAML::Value << volume.Enabled;
			
			out << YAML::Key << "Properties" << YAML::Value << YAML::BeginSeq;
			for (auto& prop : volume.Properties)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << YAML::Value << prop.second.Name;
				out << YAML::Key << "Type" << YAML::Value << (int)prop.second.Type;
				out << YAML::Key << "Data" << YAML::Value;

				switch (prop.second.Type)
				{
				case ShaderDataType::Bool: out << prop.second.Data.Bool;break;
				case ShaderDataType::Float: out << prop.second.Data.Float;break;
				case ShaderDataType::Float2: out << prop.second.Data.Vec2;break;
				case ShaderDataType::Float3: out << prop.second.Data.Vec3;break;
				case ShaderDataType::Float4: out << prop.second.Data.Vec4;break;
				case ShaderDataType::Int: out << prop.second.Data.Int;break;
				case ShaderDataType::Sampler2D: out << prop.second.Data.Texture;break;
				default: out << 0; break;
				}

				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap << YAML::EndDoc;

		outFile << out.c_str();
	}

	PostProcessingStackConfig PostProcessingStack::GetConfig(const std::string& path)
	{
		PostProcessingStackConfig config;
		std::stringstream strStream;
		std::ifstream file(path);

		if (file.good())
		{
			strStream << file.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			for (auto& volume : in["Volumes"])
			{
				PostProcessingVolume volumeStruct;
				YAML::Node properties = volume["Properties"];

				volumeStruct.Name = volume["Name"].as<std::string>();
				volumeStruct.ShaderID = volume["Shader"].as<uint64_t>();
				volumeStruct.Enabled = volume["Enabled"].as<bool>();

				properties = volume["Properties"];

				for (auto& prop : properties)
				{
					ShaderDataType type = (ShaderDataType)prop["Type"].as<int>();
					ShaderUniform uniform;

					uniform.Type = type;
					uniform.Name = prop["Name"].as<std::string>();
					switch (type)
					{
					case ShaderDataType::Bool: uniform.Data.Bool = prop["Data"].as<bool>(); break;
					case ShaderDataType::Float: uniform.Data.Float = prop["Data"].as<float>(); break;
					case ShaderDataType::Float2: uniform.Data.Vec2 = prop["Data"].as<glm::vec2>(); break;
					case ShaderDataType::Float3: uniform.Data.Vec3 = prop["Data"].as<glm::vec3>(); break;
					case ShaderDataType::Float4: uniform.Data.Vec4 = prop["Data"].as<glm::vec4>(); break;
					case ShaderDataType::Int: uniform.Data.Int = prop["Data"].as<int>(); break;
					case ShaderDataType::Sampler2D: uniform.Data.Texture = prop["Data"].as<uint64_t>(); break;
					default: break;
					}

					volumeStruct.Properties[uniform.Name] = uniform;
				}

				config.Volumes.push_back(volumeStruct);
			}

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
			config = {};

		return config;
	}

	void PostProcessingStack::SetConfig(const PostProcessingStackConfig& config)
	{
		m_Volumes = config.Volumes;
		m_ID = config.ID;

		for (auto& volume : m_Volumes)
			volume.RuntimeShader = AssetManager::Request<Shader>(volume.ShaderID);
	}
}