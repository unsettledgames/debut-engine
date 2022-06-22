#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>

#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>

namespace Debut
{
	Mesh::Mesh()
	{

	}

	template <typename T>
	static void EmitBuffer(std::vector<T> buffer, YAML::Emitter& emitter)
	{
		emitter << YAML::BeginSeq;
		for (uint32_t i = 0; i < buffer.size(); i++)
			emitter << buffer[i];
		emitter << YAML::EndSeq;
	}

	Mesh::Mesh(const std::string& path)
	{
		std::ifstream meta(path + ".meta");
		m_Path = path;

		// If the meta file exists, load its info
		if (meta.good())
		{
			std::stringstream ss;
			ss << meta.rdbuf();
			YAML::Node meta = YAML::Load(ss.str());

			m_ID = meta["ID"].as<uint64_t>();
			m_Vertices.resize(meta["NumVertices"].as<uint32_t>());
			m_Normals.resize(meta["NumVertices"].as<uint32_t>());
			m_Tangents.resize(meta["NumVertices"].as<uint32_t>());
			m_Bitangents.resize(meta["NumVertices"].as<uint32_t>());
			m_Indices.resize(meta["NumIndices"].as<uint32_t>());
			m_TexCoords.resize(meta["NumTexCoords"].as<uint32_t>());
			for (uint32_t i = 0; i < m_TexCoords.size(); i++)
				m_TexCoords[i].resize(m_Vertices.size());

			// Load rest of the model
			std::ifstream meshFile(path);
			ss.str("");
			ss << meshFile.rdbuf();
			Load(YAML::Load(ss.str()));

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
		emitter << YAML::Key << "Vertices" << YAML::Value; EmitBuffer<glm::vec3>(m_Vertices, emitter);
		emitter << YAML::Key << "Normals" << YAML::Value; EmitBuffer<glm::vec3>(m_Normals, emitter);
		emitter << YAML::Key << "Tangents" << YAML::Value; EmitBuffer<glm::vec3>(m_Tangents, emitter);
		emitter << YAML::Key << "Bitangents" << YAML::Value; EmitBuffer<glm::vec3>(m_Bitangents, emitter);
		emitter << YAML::Key << "Indices" << YAML::Value; EmitBuffer<int>(m_Indices, emitter);
		
		emitter << YAML::Key << "TexCoords" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			EmitBuffer<glm::vec2>(m_TexCoords[i], emitter);

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
		for (uint32_t i = 0; i < m_Vertices.size(); i++)
		{
			m_Vertices[i] = yaml["Vertices"][i].as<glm::vec3>();
			m_Normals[i] = yaml["Normals"][i].as<glm::vec3>();
			m_Normals[i] = yaml["Tangents"][i].as<glm::vec3>();
			m_Normals[i] = yaml["Bitangents"][i].as<glm::vec3>();
		}
		
		for (uint32_t i = 0; i < m_Indices.size(); i++)
			m_Indices[i] = yaml["Indices"][i].as<int>();

		for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			for (uint32_t j = 0; j < m_TexCoords[i].size(); j++)
				m_TexCoords[i][j] = yaml["TexCoords"][i][j].as<glm::vec3>();
	}
}