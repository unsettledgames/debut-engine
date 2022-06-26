#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>

#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>

namespace Debut
{
	Mesh::Mesh()
	{

	}

	template<typename T>
	static void EmitBuffer(std::vector<T> buffer, YAML::Emitter& emitter)
	{
		const unsigned char* byteArrayBegin = reinterpret_cast<unsigned char*>(buffer.data());
		uint32_t size = buffer.size() * sizeof(T);

		emitter << YAML::Binary(byteArrayBegin, size);
	}

	template<typename T>
	static void LoadBuffer(std::vector<T> buffer, YAML::Node& node, const std::string& name, uint32_t nElements)
	{
		YAML::Binary binaryData = node[name].as<YAML::Binary>();
		memcpy(buffer.data(), binaryData.data(), binaryData.size());
	}


	Mesh::Mesh(const std::string& path)
	{
		DBT_PROFILE_FUNCTION("Mesh::Constructor");
		std::ifstream meta(path + ".meta");
		m_Path = path;

		// If the meta file exists, load its info
		if (meta.good())
		{
			std::stringstream ss;
			ss << meta.rdbuf();
			YAML::Node meta = YAML::Load(ss.str());

			m_ID = meta["ID"].as<uint64_t>();

			float nPoints = meta["NumVertices"].as<uint32_t>();

			m_Vertices.resize(nPoints);
			m_Normals.resize(nPoints);
			m_Tangents.resize(nPoints);
			m_Bitangents.resize(nPoints);
			m_Indices.resize(meta["NumIndices"].as<uint32_t>());
			m_TexCoords.resize(meta["NumTexCoords"].as<uint32_t>());
			for (uint32_t i = 0; i < m_TexCoords.size(); i++)
				m_TexCoords[i].resize(nPoints);


			std::ifstream meshFile;
			// Load rest of the model
			{
				DBT_PROFILE_SCOPE("Mesh::OpenMeshFile");
				meshFile.open(path);
			}
			
			{
				DBT_PROFILE_SCOPE("Mesh::ReadFileBuffer");
				ss.str("");
				ss << meshFile.rdbuf();
			}

			YAML::Node node;
			{
				DBT_PROFILE_SCOPE("Mesh::YamlLoad2");
				node = YAML::Load(ss.str());
			}
			
			
			Load(node);

			m_Valid = true;
		}
		// Otherwise save the ID in a new meta file
		else
		{
			meta.close();
			std::ofstream newMeta(path + ".meta");
			YAML::Emitter emitter;
			
			emitter << YAML::BeginDoc << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << m_ID << YAML::EndMap << YAML::EndDoc;
			newMeta << emitter.c_str();
			newMeta.close();

			m_Valid = false;
		}
	}

	void Mesh::SaveSettings()
	{
		std::ofstream outFile(m_Path);
		YAML::Emitter emitter;

		emitter << YAML::BeginDoc << YAML::BeginMap;
		emitter << YAML::Key << "Name" << YAML::Value << m_Name;
		emitter << YAML::Key << "Vertices" << YAML::Value; EmitBuffer<float>(m_Vertices, emitter);
		emitter << YAML::Key << "Normals" << YAML::Value; EmitBuffer<float>(m_Normals, emitter);
		emitter << YAML::Key << "Tangents" << YAML::Value; EmitBuffer<float>(m_Tangents, emitter);
		emitter << YAML::Key << "Bitangents" << YAML::Value; EmitBuffer<float>(m_Bitangents, emitter);
		emitter << YAML::Key << "Indices" << YAML::Value; EmitBuffer<int>(m_Indices, emitter);
		
		emitter << YAML::Key << "TexCoords" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			EmitBuffer<float>(m_TexCoords[i], emitter);

		emitter << YAML::EndSeq << YAML::EndMap << YAML::EndDoc;
		outFile << emitter.c_str();
		outFile.close();

		outFile.open(m_Path + ".meta");
		YAML::Emitter metaEmitter;

		metaEmitter << YAML::BeginDoc << YAML::BeginMap;
		metaEmitter << YAML::Key << "ID" << YAML::Value << m_ID;
		metaEmitter << YAML::Key << "NumVertices" << YAML::Value << m_Vertices.size();
		metaEmitter << YAML::Key << "NumIndices" << YAML::Value << m_Indices.size();
		metaEmitter << YAML::Key << "NumTexCoords" << YAML::Value << m_TexCoords.size();
		metaEmitter << m_ID << YAML::EndMap << YAML::EndDoc;
		outFile << metaEmitter.c_str();
	}

	void Mesh::Load(YAML::Node yaml)
	{
		DBT_PROFILE_FUNCTION("Mesh:Load");
		{
			DBT_PROFILE_SCOPE("Mesh:LoadVertices");
			
			LoadBuffer<float>(m_Vertices, yaml, "Vertices", m_Vertices.size());
			LoadBuffer<float>(m_Normals, yaml, "Normals", m_Normals.size());
			LoadBuffer<float>(m_Tangents, yaml, "Tangents", m_Tangents.size());
			LoadBuffer<float>(m_Bitangents, yaml, "Bitangents", m_Bitangents.size());
		}
		
		{
			DBT_PROFILE_SCOPE("Mesh:LoadIndices");
			LoadBuffer<int>(m_Indices, yaml, "Indices", m_Indices.size());
		}

		{
			DBT_PROFILE_SCOPE("Mesh:LoadTexCoords");
			YAML::Binary binaryData;
			for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			{
				binaryData = yaml["TexCoords"][i].as<YAML::Binary>();
				memcpy(m_TexCoords[i].data(), binaryData.data(), binaryData.size());
			}
		}
	}
}