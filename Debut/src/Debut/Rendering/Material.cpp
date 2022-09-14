#include <Debut/dbtpch.h>
#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Resources/Skybox.h>

#define FIND_UNIFORM(name)	if (m_Uniforms.find(##name) == m_Uniforms.end()) \
							{	\
								/*Log.CoreError("Uniform {0} is not supported by the material {1}", ##name, m_Name);*/	\
								return;	\
							}

#define CHECK_TYPE(name, type)  ShaderUniform uniform = m_Uniforms[##name];	\
								if (uniform.Type != type)	\
								{	\
									Log.CoreWarn("The uniform {0} isn't of type Float. Try using the correct Set function.", ##name);	\
									return;	\
								}

namespace Debut
{
	UUID Material::s_PrevShader;
	std::vector<std::string> Material::s_DefaultUniforms = {
		"u_ViewProjection", "u_ViewMatrix", "u_ProjectionMatrix", "u_PointLights"
	};

	Material::Material(const std::string& path, const std::string& metaPath) : m_Path(path), m_MetaPath(metaPath)
	{
		if (m_MetaPath == "")
			m_MetaPath = m_Path + ".meta";
		// Load material if it exists, otherwise create a .meta file
		std::ifstream matFile(path);
		std::stringstream ss;

		if (matFile.good())
		{
			m_Path = path;
			Load(matFile);
		}
		else
		{
			matFile.close();
			std::string assetPath = AssetManager::s_AssetsDir + path;
			std::ifstream assetFile(assetPath);

			if (assetFile.good())
			{
				m_Path = assetPath;
				m_MetaPath = AssetManager::s_MetadataDir + path + ".meta";
				Load(assetFile);
			}
			else
			{
				assetFile.close();

				std::ofstream metaFile(m_MetaPath);

				YAML::Emitter metaEmitter;
				metaEmitter << YAML::BeginDoc << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << m_ID << YAML::EndMap << YAML::EndDoc;
				metaFile << metaEmitter.c_str();
				metaFile.close();

				// Save the default configuration for this material
				Material::SaveSettings(path, { "Untitled Material", 0, {} });
				m_Valid = false;
			}
		}
	}

	void Material::Load(std::ifstream& file)
	{
		std::stringstream ss;
		ss << file.rdbuf();
		YAML::Node inYaml = YAML::Load(ss.str());

		// Load the ID from the meta file
		std::ifstream meta(m_MetaPath);
		ss.str("");
		ss << meta.rdbuf();
		YAML::Node metaNode = YAML::Load(ss.str());
		m_ID = metaNode["ID"].as<uint64_t>();

		// First of all load the shader
		m_Shader = inYaml["Shader"].as<uint64_t>();
		m_Name = inYaml["Name"].as<std::string>();
		Ref<Shader> shader = AssetManager::Request<Shader>(m_Shader);

		if (shader == nullptr)
			return;
		// Then load the uniform fromt the shader and their values from disk
		std::vector<ShaderUniform> uniforms = shader->GetUniforms();
		YAML::Node matParams = inYaml["Parameters"];

		for (auto& uniform : uniforms)
		{
			if (matParams[uniform.Name].IsDefined())
			{
				m_Uniforms[uniform.Name] = uniform;
				switch (uniform.Type)
				{
				case ShaderDataType::Float:
					m_Uniforms[uniform.Name].Data.Float = matParams[uniform.Name].as<float>();
					break;
				case ShaderDataType::Float2:
					m_Uniforms[uniform.Name].Data.Vec2 = matParams[uniform.Name].as<glm::vec2>();
					break;
				case ShaderDataType::Float3:
					m_Uniforms[uniform.Name].Data.Vec3 = matParams[uniform.Name].as<glm::vec3>();
					break;
				case ShaderDataType::Float4:
					m_Uniforms[uniform.Name].Data.Vec4 = matParams[uniform.Name].as<glm::vec4>();
					break;
				case ShaderDataType::Mat4:
					m_Uniforms[uniform.Name].Data.Mat4 = matParams[uniform.Name].as<glm::mat4>();
					break;

				case ShaderDataType::Int:
					m_Uniforms[uniform.Name].Data.Int = matParams[uniform.Name].as<int>();
					break;
				case ShaderDataType::Bool:
					m_Uniforms[uniform.Name].Data.Bool = matParams[uniform.Name].as<bool>();
					break;
				case ShaderDataType::Sampler2D:
				{
					// TODO: add scale and offset to texture?

					// Load the texture data
					YAML::Node textureNode = matParams[uniform.Name];
					// Load the actual texture
					UUID texID = textureNode["ID"].as<uint64_t>();
					Ref<Texture2D> texture = AssetManager::Request<Texture2D>(texID);

					m_Uniforms[uniform.Name].Data.Texture = texID;
				}
				break;

				default:
					Log.CoreError("Material type {0} for uniform {1} isn't supported", (int)uniform.Type, uniform.Name);
					break;
				}
			}
		}

		m_Valid = true;
	}

	void Material::SetConfig(const MaterialConfig& config)
	{
		m_Name = config.Name;
		m_Shader = config.Shader;
		m_Uniforms = config.Uniforms;
	}

	void Material::SaveDefaultConfig(const std::string& path)
	{
		std::ofstream meta(path + ".meta");
		YAML::Emitter emitter;
		
		emitter << YAML::BeginDoc << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << UUID() << YAML::EndMap << YAML::EndDoc;
		meta << emitter.c_str();

		SaveSettings(path, { "New Material", 0, {} });
	}

	void Material::SaveSettings()
	{
		MaterialConfig config;

		config.Name = m_Name;
		config.Shader = m_Shader;
		config.Uniforms = m_Uniforms;

		SaveSettings(m_Path, config);

		// Update / create .meta file too
		YAML::Emitter metaEmitter;
		std::ofstream out(m_MetaPath);

		metaEmitter << YAML::BeginDoc << YAML::BeginMap << 
			YAML::Key << "ID" << YAML::Value << m_ID << YAML::EndMap << YAML::EndDoc;
		out << metaEmitter.c_str();
	}

	void Material::SaveSettings(const std::string& path, const MaterialConfig& config)
	{
		std::ofstream out(path);
		YAML::Emitter emitter;

		emitter << YAML::BeginDoc << YAML::BeginMap;

		emitter << YAML::Key << "Name" << YAML::Value << config.Name;
		emitter << YAML::Key << "Shader" << YAML::Value << config.Shader;
		emitter << YAML::Key << "Parameters" << YAML::Value << YAML::BeginMap;

		for (auto& uniform : config.Uniforms)
		{
			switch (uniform.second.Type)
			{
			case ShaderDataType::Float: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Float; break;
			case ShaderDataType::Float2: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Vec2; break;
			case ShaderDataType::Float3: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Vec3; break;
			case ShaderDataType::Float4: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Vec4; break;

			case ShaderDataType::Mat4: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Mat4; break;
			case ShaderDataType::Bool: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Bool; break;
			case ShaderDataType::Int: emitter << YAML::Key << uniform.second.Name << YAML::Value << uniform.second.Data.Int; break;
			case ShaderDataType::Sampler2D:
			{
				emitter << YAML::Key << uniform.second.Name << YAML::Value << YAML::BeginMap;
				emitter << YAML::Key << "ID" << YAML::Value << uniform.second.Data.Texture;
				emitter << YAML::EndMap;
				break;
			}
			default:
				break;
			}
		}

		emitter << YAML::EndMap << YAML::EndDoc;
		out << emitter.c_str();
		out.close();

	}

	void Material::Use()
	{
		// OPTIMIZABLE: Cache this?
		Ref<Shader> shader = AssetManager::Request<Shader>(m_Shader);
		if (shader->GetID() != s_PrevShader)
			shader->Bind();
		uint32_t currSlot = 0;
		
		for (auto& uniform : m_Uniforms)
		{
			switch (uniform.second.Type)
			{
			case ShaderDataType::Float:
				shader->SetFloat(uniform.second.Name, uniform.second.Data.Float);
				break;
			case ShaderDataType::Float3:
				shader->SetFloat3(uniform.second.Name, uniform.second.Data.Vec3);
				break;
			case ShaderDataType::Float4:
				shader->SetFloat4(uniform.second.Name, uniform.second.Data.Vec4);
				break;
			case ShaderDataType::Mat4:
				shader->SetMat4(uniform.second.Name, uniform.second.Data.Mat4);
				break;
			case ShaderDataType::Sampler2D:
			{
				Ref<Texture2D> texture;
				if (uniform.second.Data.Texture != 0)
					texture = AssetManager::Request<Texture2D>(uniform.second.Data.Texture);
				else
					texture = AssetManager::Request<Texture2D>(DBT_WHITE_TEXTURE_UUID);

				texture->Bind(currSlot);
				currSlot++;
				break;
			}
			case ShaderDataType::SamplerCube:
				shader->SetInt(uniform.second.Name, uniform.second.Data.Cubemap);
				break;
			default:
				Log.CoreError("Shader data type not supported while trying to use material {0}", m_Name);
				break;
			}
		}
	}

	void Material::Unuse()
	{
		Ref<Shader> shader = AssetManager::Request<Shader>(m_Shader);
		shader->Unbind();
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		// Set the shader and the new uniforms
		auto uniforms = shader->GetUniforms();
		m_Shader = shader->GetID();
		
		m_Uniforms.clear();
		for (auto uniform : uniforms)
			m_Uniforms[uniform.Name] = uniform;
	}

	void Material::SetFloat(const std::string& name, float val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float);
		m_Uniforms[name].Data.Float = val;
	}

	void Material::SetVec2(const std::string& name, const glm::vec2& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float2);
		m_Uniforms[name].Data.Vec2 = vec;
	}

	void Material::SetVec3(const std::string& name, const glm::vec3& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float3);
		m_Uniforms[name].Data.Vec3 = vec;
	}

	void Material::SetVec4(const std::string& name, const glm::vec4& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float4);
		m_Uniforms[name].Data.Vec4 = vec;
	}

	void Material::SetMat4(const std::string& name, const glm::mat4& mat)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Mat4);
		m_Uniforms[name].Data.Mat4 = mat;
	}

	void Material::SetInt(const std::string& name, int val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Int);
		m_Uniforms[name].Data.Int = val;
	}

	void Material::SetBool(const std::string& name, bool val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Bool);
		m_Uniforms[name].Data.Bool = val;
	}

	void Material::SetTexture(const std::string& name, const Ref<Texture2D> texture)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Sampler2D);
		m_Uniforms[name].Data.Texture = texture->GetID();
	}

	void Material::SetCubemap(const std::string& name, const Ref<Skybox> cubemap)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::SamplerCube);
		m_Uniforms[name].Data.Cubemap = cubemap->GetRendererID();
	}

	void Material::Reload()
	{
		std::ifstream matFile(m_Path);
		Load(matFile);
	}

	MaterialMetadata Material::GetMetadata(UUID id)
	{
		MaterialMetadata ret = {};
		std::string path = AssetManager::GetPath(id);
		std::ifstream metaFile(path);

		if (!metaFile.good())
		{
			std::stringstream ss;
			ss << AssetManager::s_AssetsDir << id;

			metaFile.open(ss.str());
		}

		if (metaFile.good())
		{
			std::stringstream ss;
			ss << metaFile.rdbuf();

			YAML::Node metaNode = YAML::Load(ss.str());
			ret.Name = metaNode["Name"].as<std::string>();
			ret.ID = id;
		}

		return ret;
	}
}