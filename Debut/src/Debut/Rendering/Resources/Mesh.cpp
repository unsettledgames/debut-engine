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

		// If the meta file exists, load its info
		if (meta.good())
		{
			std::stringstream ss;
			ss << meta.rdbuf();
			YAML::Node meta = YAML::Load(ss.str());

			m_ID = meta["ID"].as<uint64_t>();

			// Load rest of the model
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

			// Save vectors
		}
	}

	void Mesh::SaveSettings()
	{
		/* 
		 * Save mesh file:
		 *	- Vertices
		 *	- Normals
		 *  - Tangents
		 *	- Bitangents
		 *  - TexCoords
		*/
		std::ofstream outFile(m_Path);
		YAML::Emitter emitter;

		emitter << YAML::BeginDoc << YAML::BeginMap;
		emitter << YAML::Key << "Name" << YAML::Value << m_Name;
		emitter << YAML::Key << "Vertices" << YAML::Value << YAML::Binary((unsigned char*)m_Vertices.data(), sizeof(m_Vertices.data()));
		emitter << YAML::Key << "Normals" << YAML::Value << YAML::Binary((unsigned char*)m_Normals.data(), sizeof(m_Normals.data()));
		emitter << YAML::Key << "Tangents" << YAML::Value << YAML::Binary((unsigned char*)m_Tangents.data(), sizeof(m_Tangents.data()));
		emitter << YAML::Key << "Bitangents" << YAML::Value << YAML::Binary((unsigned char*)m_Bitangents.data(), sizeof(m_Bitangents.data()));
		emitter << YAML::Key << "Indices" << YAML::Value << YAML::Binary((unsigned char*)m_Indices.data(), sizeof(m_Indices.data()));
	}
}