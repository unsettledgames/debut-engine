#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>

#include <yaml-cpp/yaml.h>

namespace Debut
{
	Mesh::Mesh()
	{

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
			Load(YAML::Load(path));

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
		emitter << YAML::Key << "Vertices" << YAML::Value << YAML::Binary((unsigned char*)m_Vertices.data(), sizeof(m_Vertices.data()));
		emitter << YAML::Key << "Normals" << YAML::Value << YAML::Binary((unsigned char*)m_Normals.data(), sizeof(m_Normals.data()));
		emitter << YAML::Key << "Tangents" << YAML::Value << YAML::Binary((unsigned char*)m_Tangents.data(), sizeof(m_Tangents.data()));
		emitter << YAML::Key << "Bitangents" << YAML::Value << YAML::Binary((unsigned char*)m_Bitangents.data(), sizeof(m_Bitangents.data()));
		emitter << YAML::Key << "Indices" << YAML::Value << YAML::Binary((unsigned char*)m_Indices.data(), sizeof(m_Indices.data()));
		
		emitter << YAML::Key << "TexCoords" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			emitter << YAML::Binary((unsigned char*)m_TexCoords[i].data(), sizeof(m_TexCoords[i].data()));

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
		YAML::Binary binaryData = yaml["Vertices"].as<YAML::Binary>();
		memcpy(m_Vertices.data(), (const void*)binaryData.data(), binaryData.size());

		binaryData = yaml["Normals"].as<YAML::Binary>();
		memcpy(m_Normals.data(), (const void*)binaryData.data(), binaryData.size());

		binaryData = yaml["Tangents"].as<YAML::Binary>();
		memcpy(m_Tangents.data(), (const void*)binaryData.data(), binaryData.size());

		binaryData = yaml["Bitangents"].as<YAML::Binary>();
		memcpy(m_Bitangents.data(), (const void*)binaryData.data(), binaryData.size());

		binaryData = yaml["Indices"].as<YAML::Binary>();
		memcpy(m_Indices.data(), (const void*)binaryData.data(), binaryData.size());

		for (uint32_t i = 0; i < m_TexCoords.size(); i++)
		{
			binaryData = yaml["TexCoords"][i].as<YAML::Binary>();
			memcpy(m_TexCoords[i].data(), (const void*)binaryData.data(), binaryData.size());
		}
	}
}