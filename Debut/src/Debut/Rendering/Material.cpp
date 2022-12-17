#include <Debut/dbtpch.h>
#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>
#include <Debut/Utils/CppUtils.h>
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
	UUID Material::s_PrevShader = 0;

	std::vector<std::string> Material::s_DefaultUniforms = {
		"u_ViewProjection", "u_ViewMatrix", "u_ProjectionMatrix", "u_PointLights", "u_AmbientLightColor", "u_DirectionalLightDir",
		"u_AmbientLightIntensity", "u_CameraPosition", "u_DirectionalLightCol", "u_DirectionalLightIntensity"
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
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<float>();
					break;
				case ShaderDataType::Float2:
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<glm::vec2>();
					break;
				case ShaderDataType::Float3:
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<glm::vec3>();
					break;
				case ShaderDataType::Float4:
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<glm::vec4>();
					break;
				case ShaderDataType::Mat4:
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<glm::mat4>();
					break;

				case ShaderDataType::Int:
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<int>();
					break;
				case ShaderDataType::Bool:
					m_Uniforms[uniform.Name].Data = matParams[uniform.Name].as<bool>();
					break;
				case ShaderDataType::Sampler2D:
				{
					// Load the texture data
					YAML::Node textureNode = matParams[uniform.Name];
					// Load the actual texture
					UUID texID = textureNode["ID"].as<uint64_t>();
					Ref<Texture2D> texture = AssetManager::Request<Texture2D>(texID);

					m_Uniforms[uniform.Name].Data = texID;
				}
				break;

				default:
					Log.CoreError("Material type {0} for uniform {1} isn't supported", (int)uniform.Type, uniform.Name);
					break;
				}
			}
		}

		m_RuntimeShader = AssetManager::Request<Shader>(m_Shader);
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
			case ShaderDataType::Float: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<float>(uniform.second.Data); break;
			case ShaderDataType::Float2: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<glm::vec2>(uniform.second.Data); break;
			case ShaderDataType::Float3: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<glm::vec3>(uniform.second.Data); break;
			case ShaderDataType::Float4: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<glm::vec4>(uniform.second.Data); break;

			case ShaderDataType::Mat4: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<glm::mat4>(uniform.second.Data); break;
			case ShaderDataType::Bool: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<bool>(uniform.second.Data); break;
			case ShaderDataType::Int: emitter << YAML::Key << uniform.second.Name << YAML::Value << std::get<int>(uniform.second.Data); break;
			case ShaderDataType::Sampler2D:
			{
				emitter << YAML::Key << uniform.second.Name << YAML::Value << YAML::BeginMap;
				emitter << YAML::Key << "ID" << YAML::Value << std::get<UUID>(uniform.second.Data);
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
		/* This could work, but the bound shader should be in some kind of RendererState class: the shader can be
		*  changed by other renderers
		if (s_PrevShader != m_RuntimeShader->GetID())
		{
			m_RuntimeShader->Bind();
			s_PrevShader = m_RuntimeShader->GetID();
		}
		*/
		if (m_RuntimeShader == nullptr)
			m_RuntimeShader = AssetManager::Request<Shader>(m_Shader);
		if (m_RuntimeShader == nullptr)
			return;
		m_RuntimeShader->Bind();
		uint32_t currSlot = 0;
		
		for (auto& uniform : m_Uniforms)
		{
			switch (uniform.second.Type)
			{
			case ShaderDataType::Int:
				m_RuntimeShader->SetInt(uniform.second.Name, std::get<int>(uniform.second.Data));
				break;
			case ShaderDataType::Bool:
			{
				//DBT_PROFILE_SCOPE("Material::SetBool");
				m_RuntimeShader->SetBool(uniform.second.Name, std::get<bool>(uniform.second.Data));
				break;
			}
			case ShaderDataType::Float:
			{
				//DBT_PROFILE_SCOPE("Material::SetFloat");
				m_RuntimeShader->SetFloat(uniform.second.Name, std::get<float>(uniform.second.Data));
				break;
			}
			case ShaderDataType::Float2:
			{
				//DBT_PROFILE_SCOPE("Material::SetFloat2");
				m_RuntimeShader->SetFloat2(uniform.second.Name, std::get<glm::vec2>(uniform.second.Data));
				break;
			}
			case ShaderDataType::Float3:
			{
				//DBT_PROFILE_SCOPE("Material::SetFloat3");
				m_RuntimeShader->SetFloat3(uniform.second.Name, std::get<glm::vec3>(uniform.second.Data));
				break;
			}
			case ShaderDataType::Float4:
			{
				//DBT_PROFILE_SCOPE("Material::SetFloat4");
				m_RuntimeShader->SetFloat4(uniform.second.Name, std::get<glm::vec4>(uniform.second.Data));
				break;
			}
			case ShaderDataType::Mat4:
			{
				//DBT_PROFILE_SCOPE("Material::SetMat4");
				m_RuntimeShader->SetMat4(uniform.second.Name, std::get<glm::mat4>(uniform.second.Data));
				break;
			}
			case ShaderDataType::Sampler2D:
			{
				//DBT_PROFILE_SCOPE("Material::SetTexture");
				Ref<Texture2D> texture;
				UUID texID = std::get<UUID>(uniform.second.Data);
				if (m_RuntimeTextures.find(texID) == m_RuntimeTextures.end())
					if (texID != 0)
						m_RuntimeTextures[texID] = AssetManager::Request<Texture2D>(texID);
					else
						m_RuntimeTextures[texID] = AssetManager::Request<Texture2D>(DBT_WHITE_TEXTURE_UUID);

				texture = m_RuntimeTextures[texID];

				m_RuntimeShader->SetInt(uniform.second.Name, currSlot);
				texture->Bind(currSlot);
				currSlot++;
				break;
			}
			case ShaderDataType::SamplerCube:
				m_RuntimeShader->SetInt(uniform.second.Name, std::get<UUID>(uniform.second.Data));
				break;
			case ShaderDataType::None:
				break;
			default:
				Log.CoreError("Shader data type for uniform {0} not supported while trying to use material {1}", uniform.second.Name, m_Name);
				break;
			}
		}

		m_CurrTextureSlot = currSlot;
	}

	void Material::Unuse()
	{
		m_RuntimeShader->Unbind();
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		DBT_PROFILE_FUNCTION();
		if (shader == nullptr)
			return;
		// Set the shader and the new uniforms
		auto uniforms = shader->GetUniforms();
		m_Shader = shader->GetID();
		m_RuntimeShader = AssetManager::Request<Shader>(m_Shader);
		
		{
			DBT_PROFILE_SCOPE("SetUniformMap");
			m_Uniforms.clear();
			for (auto uniform : uniforms)
				m_Uniforms[uniform.Name] = uniform;
		}
	}

	void Material::SetFloat(const std::string& name, float val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float);
		m_Uniforms[name].Data = val;
	}

	void Material::SetVec2(const std::string& name, const glm::vec2& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float2);
		m_Uniforms[name].Data = vec;
	}

	void Material::SetVec3(const std::string& name, const glm::vec3& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float3);
		m_Uniforms[name].Data = vec;
	}

	void Material::SetVec4(const std::string& name, const glm::vec4& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float4);
		m_Uniforms[name].Data = vec;
	}

	void Material::SetMat4(const std::string& name, const glm::mat4& mat)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Mat4);
		m_Uniforms[name].Data = mat;
	}

	void Material::SetInt(const std::string& name, int val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Int);
		m_Uniforms[name].Data = val;
	}

	void Material::SetBool(const std::string& name, bool val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Bool);
		m_Uniforms[name].Data = val;
	}

	void Material::SetTexture(const std::string& name, const Ref<Texture2D> texture)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Sampler2D);
		m_Uniforms[name].Data = texture->GetID();
	}

	void Material::SetCubemap(const std::string& name, const Ref<Skybox> cubemap)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::SamplerCube);
		// ISSUE: don't use UUID, use uint32_t
		m_Uniforms[name].Data = (UUID)cubemap->GetRendererID();
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