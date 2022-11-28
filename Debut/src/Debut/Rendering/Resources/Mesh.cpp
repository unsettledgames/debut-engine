#include <Debut/dbtpch.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h> 
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/AssetManager/AssetManager.h>

#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>
#include <lz4.h>

namespace Debut
{
	Mesh::Mesh()
	{
		m_Valid = false;
	}

	Mesh::~Mesh()
	{

	}

	template<typename T>
	static void EmitBuffer(std::vector<T>& buffer, std::ofstream& file)
	{
		if (buffer.size() == 0)
		{
			file << 0 << "\n";
			file << false << "\n";
			return;
		}

		size_t uncompressedSize = sizeof(T) * buffer.size();
		size_t compressBound = LZ4_compressBound(uncompressedSize);
		size_t compressedSize;

		const char* byteArrayBegin = reinterpret_cast<char*>(buffer.data());

		// Don't compress if the compression produces more data than before
		if (compressBound > uncompressedSize)
		{
			file << uncompressedSize << "\n";
			file << false << "\n";
			file.write(byteArrayBegin, uncompressedSize);
		}
		else
		{
			char* compressedByteArray = new char[compressBound];
			memset(compressedByteArray, 0, uncompressedSize);

			{
				DBT_PROFILE_SCOPE("SaveMesh::CompressBuffer");
				compressedSize = LZ4_compress_default(byteArrayBegin, compressedByteArray, uncompressedSize, compressBound);

				if (compressedSize == 0)
					Log.CoreError("Compression error: {0}", compressedSize);
			}

			{
				DBT_PROFILE_SCOPE("SaveMesh::SaveToFile");
				file << true << "\n";
				file << compressedSize << "\n";
				file.write((const char*)compressedByteArray, compressedSize);

				delete[] compressedByteArray;
			}
		}		
	}

	template<typename T>
	static void LoadBuffer(std::vector<T>& buffer, std::ifstream& file, uint32_t nElements)
	{
		std::string string = "";
		std::string compressedString;
		std::string hasCompressedStr;

		bool compressed;
		size_t compressedSize;
		size_t decompressedSize;

		while (string.compare("") == 0 || string.compare(" ") == 0)
			std::getline(file, string);	
		std::getline(file, compressedString);
		std::getline(file, hasCompressedStr);

		compressed = std::stoi(hasCompressedStr);
		compressedSize = std::stoi(compressedString);

		if (compressedSize == 0)
			return;

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

		if (compressed)
		{
			char* dst = (char*)malloc(sizeof(T) * nElements);
			decompressedSize = LZ4_decompress_safe((const char*)buffer.data(), dst, compressedSize, sizeof(T) * nElements);
			memcpy(buffer.data(), dst, sizeof(T) * nElements);
			free(dst);

			if (decompressedSize < 0)
				Log.CoreError("Decompression error");
		}
	}


	Mesh::Mesh(const std::string& path, const std::string& metaPath) : m_Path(path), m_MetaPath(metaPath)
	{
		DBT_PROFILE_FUNCTION();
		
		std::ifstream meta(m_MetaPath);
		// If the meta file exists, load its info
		if (meta.good())
		{
			std::stringstream ss;
			ss << meta.rdbuf();
			YAML::Node meta = YAML::Load(ss.str());

			m_ID = meta["ID"].as<uint64_t>();
			m_Name = meta["Name"].as<std::string>();

			m_NumVertices = meta["NumVertices"].as<uint32_t>();
			m_NumIndices = meta["NumIndices"].as<uint32_t>();
			m_NumTexCoords = meta["NumTexCoords"].as<uint32_t>();

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

	void Mesh::SaveSettings(std::vector<float>& positions, std::vector<float>& colors, std::vector<float>& normals,
		std::vector<float>& tangents, std::vector<float>& bitangents, std::vector<std::vector<float>>& texcoords,
		std::vector<int>& indices)
	{
		std::ofstream outFile(m_Path, std::ios::out | std::ios::binary);
		std::stringstream ss;
		ss << AssetManager::s_MetadataDir << m_ID << ".meta";
		std::string metaPath = ss.str();

		m_NumVertices = positions.size();
		m_NumIndices = indices.size();
		m_NumTexCoords = texcoords.size();

		{
			DBT_PROFILE_SCOPE("SaveMesh::EmitBuffers");

			for (uint32_t i = 0; i < 4; i++)
				for (uint32_t j=0; j<4; j++)
					outFile << m_Transform[i][j] << " ";
			outFile << "\n";

			Log.CoreInfo("Vertices");
			outFile << "Vertices" << "\n"; EmitBuffer<float>(positions, outFile);
			Log.CoreInfo("Colors");
			outFile << "\nColors\n"; EmitBuffer<float>(colors, outFile);
			Log.CoreInfo("Normals");
			outFile << "\nNormals" << "\n"; EmitBuffer<float>(normals, outFile);
			Log.CoreInfo("Tangents");
			outFile << "\nTangents" << "\n"; EmitBuffer<float>(tangents, outFile);
			Log.CoreInfo("Bitangents");
			outFile << "\nBitangents" << "\n"; EmitBuffer<float>(bitangents, outFile);
			Log.CoreInfo("Indices");
			outFile << "\nIndices" << "\n"; EmitBuffer<int>(indices, outFile);
			Log.CoreInfo("TexCoords");
			outFile << "\nTexCoords" << "\n";
			for (uint32_t i = 0; i < texcoords.size(); i++)
			{
				outFile << "\nTexCoords" + i << "\n";
				EmitBuffer<float>(texcoords[i], outFile);
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
			metaEmitter << YAML::Key << "NumVertices" << YAML::Value << positions.size();
			metaEmitter << YAML::Key << "NumIndices" << YAML::Value << indices.size();
			metaEmitter << YAML::Key << "NumTexCoords" << YAML::Value << texcoords.size();
			metaEmitter << m_ID << YAML::EndMap << YAML::EndDoc;
			outFile << metaEmitter.c_str();
		}
	}

	void Mesh::Load(const std::string& path)
	{
		std::ifstream file(path);
		Load(file);
	}

	void Mesh::Load(std::ifstream& inFile)
	{
		// Load buffers from disk
		std::vector<float> positions(m_NumVertices), colors((m_NumVertices / 3) * 4), normals(m_NumVertices),
			tangents(m_NumVertices), bitangents(m_NumVertices), texCoords(m_NumVertices);
		std::vector<int> indices(m_NumIndices), entityIDs(m_NumVertices / 3);

		{
			DBT_PROFILE_SCOPE("Mesh::LoadBuffers");

			for (uint32_t i = 0; i < 4; i++)
				for (uint32_t j = 0; j < 4; j++)
					inFile >> m_Transform[i][j];
			LoadBuffer<float>(positions, inFile, positions.size());
			LoadBuffer<float>(colors, inFile, colors.size());
			LoadBuffer<float>(normals, inFile, normals.size());
			LoadBuffer<float>(tangents, inFile, tangents.size());
			LoadBuffer<float>(bitangents, inFile, bitangents.size());
			LoadBuffer<int>(indices, inFile, indices.size());

			if (m_NumTexCoords > 0)
			{
				std::string texString;
				inFile >> texString;
				LoadBuffer<float>(texCoords, inFile, texCoords.size());
			}

			GenerateAABB(positions);

			m_Vertices = positions;
			m_Indices = indices;
		}
	
		// Create runtime structures
		m_VertexArray = VertexArray::Create();
		m_IndexBuffer = IndexBuffer::Create();

		{
			DBT_PROFILE_SCOPE("Mesh::CreateRuntimeBuffers");
			// Create and attach buffers
			ShaderDataType types[] = { ShaderDataType::Float3, ShaderDataType::Float4, ShaderDataType::Float3,
				ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2/*, ShaderDataType::Int*/ };
			std::string attribNames[] = { "a_Position", "a_Color", "a_Normal", "a_Tangent", "a_Bitangent", "a_TexCoords0"/*, "a_EntityID" */ };
			std::string names[] = { "Positions", "Colors", "Normals", "Tangents", "Bitangents", "TexCoords0" /*, "EntityID"*/ };
			std::vector<std::vector<float>> floatBuffers = { positions, colors, normals, tangents, bitangents, texCoords };

			for (uint32_t i = 0; i < floatBuffers.size(); i++)
			{
				m_VertexBuffers[names[i]] = VertexBuffer::Create(floatBuffers[i].data(), floatBuffers[i].size());
				m_VertexBuffers[names[i]]->SetLayout({ {types[i], attribNames[i], false} });
				m_VertexArray->AddVertexBuffer(m_VertexBuffers[names[i]]);
			}

			m_IndexBuffer->SetData(indices.data(), indices.size());
			m_VertexArray->AddIndexBuffer(m_IndexBuffer);
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

	void Mesh::GenerateAABB(const std::vector<float>& vertices)
	{
		DBT_PROFILE_SCOPE("GenerateAABB");
		m_AABB.Center = glm::vec3(0.0f);

		// Compute mesh bounds
		float xBounds[2], yBounds[2], zBounds[2];
		xBounds[0] = std::numeric_limits<float>().max(); xBounds[1] = -std::numeric_limits<float>().max();
		yBounds[0] = xBounds[0]; yBounds[1] = xBounds[1];
		zBounds[0] = xBounds[0]; zBounds[1] = xBounds[1];

		for (uint32_t i = 0; i < vertices.size(); i += 3)
		{
			float x = vertices[i], y = vertices[i + 1], z = vertices[i + 2];
			xBounds[0] = std::min(xBounds[0], x); xBounds[1] = std::max(xBounds[1], x);
			yBounds[0] = std::min(yBounds[0], y); yBounds[1] = std::max(yBounds[1], y);
			zBounds[0] = std::min(zBounds[0], z); zBounds[1] = std::max(zBounds[1], z);
		}

		m_AABB.MaxExtents = { xBounds[1], yBounds[1], zBounds[1] };
		m_AABB.MinExtents = { xBounds[0], yBounds[0], zBounds[0] };
		m_AABB.Center = m_Transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	void Mesh::GenerateBuffers()
	{

	}
}