#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/AssetManager/AssetManager.h>

#include <lz4.h>

#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>

namespace Debut
{
	Mesh::Mesh()
	{
		m_Valid = false;
	}

	template<typename T>
	static void EmitBuffer(std::vector<T>& buffer, std::ofstream& file)
	{
		size_t uncompressedSize = sizeof(T) * buffer.size();
		size_t compressedSize;

		const char* byteArrayBegin = reinterpret_cast<char*>(buffer.data());
		char* compressedByteArray = new char[uncompressedSize];
		
		{
			DBT_PROFILE_SCOPE("SaveMesh::CompressBuffer");
			Log.CoreInfo("Compressing");
			compressedSize = LZ4_compress_default(byteArrayBegin, compressedByteArray, uncompressedSize, uncompressedSize);
			if (compressedSize == 0)
				Log.CoreError("Compression error");
		}
		
		{
			DBT_PROFILE_SCOPE("SaveMesh::SaveToFile");
			file << compressedSize << "\n";
			file.write((const char*)compressedByteArray, compressedSize);

			delete[] compressedByteArray;
		}
	}

	template<typename T>
	static void LoadBuffer(std::vector<T>& buffer, std::ifstream& file, uint32_t nElements)
	{
		std::string string = "";
		std::string compressedString;

		size_t compressedSize;
		size_t decompressedSize;

		while (string.compare("") == 0)
			std::getline(file, string);	
		std::getline(file, compressedString);
		compressedSize = std::stoi(compressedString);

		file.read(reinterpret_cast<char*>(buffer.data()), compressedSize);
		if (file)
			Log.CoreInfo("Read successful");
		else
		{
			Log.CoreInfo("Read unsuccessful (read {0})", file.gcount());
			Log.CoreInfo("Open? {0}", file.is_open());
			Log.CoreInfo("Good? {0}", file.good());
			Log.CoreInfo("Eof? {0}", file.eof());
			Log.CoreInfo("Fail? {0}", file.fail());
			Log.CoreInfo("Bad? {0}", file.bad());
		}

		char* dst = (char*)malloc(sizeof(T) * nElements);
		decompressedSize = LZ4_decompress_safe((const char*)buffer.data(), dst, compressedSize, sizeof(T) * nElements);
		memcpy(buffer.data(), dst, sizeof(T) * nElements);
		free(dst);
	}


	Mesh::Mesh(const std::string& path, const std::string& metaPath) : m_Path(path), m_MetaPath(metaPath)
	{
		DBT_PROFILE_FUNCTION("Mesh::Constructor");
		
		std::ifstream meta(m_MetaPath);
		// If the meta file exists, load its info
		if (meta.good())
		{
			std::stringstream ss;
			ss << meta.rdbuf();
			YAML::Node meta = YAML::Load(ss.str());

			m_ID = meta["ID"].as<uint64_t>();
			m_Name = meta["Name"].as<std::string>();

			float nPoints = meta["NumVertices"].as<uint32_t>();

			m_Vertices.resize(nPoints);
			m_Normals.resize(nPoints);
			m_Tangents.resize(nPoints);
			m_Bitangents.resize(nPoints);
			m_Indices.resize(meta["NumIndices"].as<uint32_t>());
			m_TexCoords.resize(meta["NumTexCoords"].as<uint32_t>());
			for (uint32_t i = 0; i < m_TexCoords.size(); i++)
				m_TexCoords[i].resize(nPoints);

			std::ifstream meshFile(m_Path, std::ios::in | std::ios::binary);
			Load(meshFile);

			m_Valid = true;
		}
		// Otherwise load from the assets folder
		else
		{
			meta.close();

			std::string assetFile = AssetManager::s_AssetsDir + path;
			std::ifstream meshFile(assetFile);
			if (meshFile.good())
			{
				Load(meshFile);
			}
			else
			{
				meshFile.close();
				std::ofstream newMeta(m_MetaPath);
				YAML::Emitter emitter;

				emitter << YAML::BeginDoc << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << m_ID << YAML::EndMap << YAML::EndDoc;
				newMeta << emitter.c_str();
				newMeta.close();

				m_Valid = false;
			}
		}
	}

	void Mesh::SaveSettings()
	{
		std::ofstream outFile(m_Path, std::ios::out | std::ios::binary);
		std::stringstream ss;
		ss << AssetManager::s_MetadataDir << m_ID << ".meta";
		std::string metaPath = ss.str();

		{
			DBT_PROFILE_SCOPE("SaveMesh::EmitBuffers");
			outFile << "Vertices" << "\n"; EmitBuffer<float>(m_Vertices, outFile);
			outFile << "\nNormals" << "\n"; EmitBuffer<float>(m_Normals, outFile);
			outFile << "\nTangents" << "\n"; EmitBuffer<float>(m_Tangents, outFile);
			outFile << "\nBitangents" << "\n"; EmitBuffer<float>(m_Bitangents, outFile);
			outFile << "\nIndices" << "\n"; EmitBuffer<int>(m_Indices, outFile);
			outFile << "\nTexCoords" << "\n";
			for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			{
				outFile << "\nTexCoords" + i << "\n";
				EmitBuffer<float>(m_TexCoords[i], outFile);
			}
		}

		outFile.close();
		DBT_PROFILE_SCOPE("SaveMesh::SaveMeta")
		{
			outFile.open(metaPath);
			YAML::Emitter metaEmitter;

			metaEmitter << YAML::BeginDoc << YAML::BeginMap;
			metaEmitter << YAML::Key << "ID" << YAML::Value << m_ID;
			metaEmitter << YAML::Key << "Name" << YAML::Value << m_Name;
			metaEmitter << YAML::Key << "NumVertices" << YAML::Value << m_Vertices.size();
			metaEmitter << YAML::Key << "NumIndices" << YAML::Value << m_Indices.size();
			metaEmitter << YAML::Key << "NumTexCoords" << YAML::Value << m_TexCoords.size();
			metaEmitter << m_ID << YAML::EndMap << YAML::EndDoc;
			outFile << metaEmitter.c_str();
		}
	}

	void Mesh::Load(std::ifstream& inFile)
	{
		DBT_PROFILE_FUNCTION("Mesh:Load");
		{
			Log.CoreInfo("Load {0} vertices", m_Vertices.size());
			LoadBuffer<float>(m_Vertices, inFile, m_Vertices.size());
			Log.CoreInfo("Load {0} normals", m_Normals.size());
			LoadBuffer<float>(m_Normals, inFile, m_Normals.size());
			Log.CoreInfo("Load {0} tangents", m_Tangents.size());
			LoadBuffer<float>(m_Tangents, inFile, m_Tangents.size());
			Log.CoreInfo("Load {0} bitangents", m_Bitangents.size());
			LoadBuffer<float>(m_Bitangents, inFile, m_Bitangents.size());
			Log.CoreInfo("Load {0} indices", m_Indices.size());
			LoadBuffer<int>(m_Indices, inFile, m_Indices.size());

			std::string texString;
			inFile >> texString;
			for (uint32_t i = 0; i < m_TexCoords.size(); i++)
			{
				Log.CoreInfo("Load {0} texcoord {1}", m_TexCoords.size(), i);
				LoadBuffer<float>(m_TexCoords[i], inFile, m_TexCoords[i].size());
			}
		}
	}

	MeshMetadata Mesh::GetMetadata(UUID id)
	{
		MeshMetadata ret = {};
		std::stringstream ss;
		ss << AssetManager::s_MetadataDir << id << ".meta";
		std::ifstream metaFile(ss.str());
		
		if (metaFile.good())
		{
			std::stringstream ss;
			ss << metaFile.rdbuf();

			ret.ID = id;
			ret.Name = YAML::Load(ss.str())["Name"].as<std::string>();
		}

		return ret;
	}
}