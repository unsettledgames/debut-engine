#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/AssetManager/AssetManager.h>

#include <brotli/encode.h>
#include <brotli/decode.h>

#include <corto/encoder.h>
#include <corto/decoder.h>

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

		const unsigned char* byteArrayBegin = reinterpret_cast<unsigned char*>(buffer.data());
		unsigned char* compressedByteArray = new unsigned char[uncompressedSize];
		
		{
			DBT_PROFILE_SCOPE("SaveMesh::CompressBuffer");
			if (BrotliEncoderCompress(BROTLI_MAX_QUALITY, BROTLI_DEFAULT_WINDOW, BrotliEncoderMode::BROTLI_MODE_GENERIC,
				buffer.size() * sizeof(T), byteArrayBegin, &compressedSize, compressedByteArray) == BROTLI_FALSE)
				Log.CoreError("Mesh compression error");
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
		std::string string;

		size_t compressedSize;
		size_t decompressedSize;

		file >> string;
		file >> compressedSize;
		file.read(reinterpret_cast<char*>(buffer.data()), compressedSize + sizeof(T));

		BrotliDecoderResult ret = BrotliDecoderDecompress(compressedSize, (const uint8_t*)buffer.data(), &decompressedSize, (uint8_t*)buffer.data());
		if (ret != 1)
			Log.CoreError("Mesh decompression error {0}", ret);
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