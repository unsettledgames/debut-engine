#include <imgui_internal.h>
#include <Debut/dbtpch.h>
#include <Debut/Utils/CppUtils.h>
#include <Debut/ImGui/ImGuiUtils.h>
#include <yaml-cpp/yaml.h>
#include <stack>

#include "PropertiesPanel.h"
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>

#include <Debut/Rendering/Resources/Skybox.h>
#include <Debut/Rendering/Resources/PostProcessing.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Physics/PhysicsMaterial3D.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Rendering/Texture.h>

/**
	TODO:
	- Allow renaming for each asset
*/

namespace Debut
{
	static std::vector<std::string> s_SupportedExtensions = { ".png", ".physmat2d", ".physmat3d", ".glsl", ".mat", ".model", 
		".skybox", ".postps", ".jpg", ".tga"};
	static std::vector<std::string> s_ModelExtensions = { ".obj", ".fbx", ".dae", ".gltf", ".glb", ".blend", ".3ds", ".ase",
		".ifc", ".xgl", ".zgl", ".ply", ".dxf", ".lwo", ".lws", ".lxo", ".stl", ".x", ".ac", ".ms3d", ".cob", ".scn" };

	static int SetFileName(ImGuiInputTextCallbackData* data)
	{
		int sas = 2;
		return 0;
	}

	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::Begin("Properties");
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 2, 2 });

		if (m_AssetPath != "")
		{
			if (std::find(s_SupportedExtensions.begin(), s_SupportedExtensions.end(), m_AssetPath.extension().string()) != s_SupportedExtensions.end())
			{
				DrawName();
			}

			// Put extensions as static members of Texture2D and PhysicsMaterial2D?
			if (m_AssetType == AssetType::Texture2D)
			{
				DrawTextureProperties();
			}
			else if (m_AssetType == AssetType::Skybox)
			{
				DrawSkyboxProperties();
			}
			else if (m_AssetType == AssetType::PostProcessingStack)
			{
				DrawPostProcessingStackProperties();
			}
			else if (m_AssetType == AssetType::PhysicsMaterial2D)
			{
				DrawPhysicsMaterial2DProperties();
			}
			else if (m_AssetType == AssetType::PhysicsMaterial3D)
			{
				DrawPhysicsMaterial3DProperties();
			}
			else if (m_AssetType == AssetType::Shader)
			{
				DrawShaderProperties();
			}
			else if (m_AssetType == AssetType::Material)
			{
				DrawMaterialProperties();
			}
			else if (m_AssetType == AssetType::Model)
			{
				DrawModelProperties();
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();

		m_PrevAssetPath = m_AssetPath;
	}

	void PropertiesPanel::DrawName()
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::TextWrapped("Import settings");
		ImGui::PopItemWidth();
		ImGui::PopFont();

		char renameName[128];
		wcstombs(renameName, m_AssetPath.filename().c_str(), m_AssetPath.string().length());

		ImGui::PushID(m_AssetPath.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100);
		ImGui::SetColumnWidth(1, ImGui::GetWindowWidth() - 100);

		ImGui::TextWrapped("Name");
		ImGui::NextColumn();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() - 100);


		if (ImGui::InputText("##", renameName, 128, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// TODO: Set name and edit it in the asset manager
		}

		ImGui::PopItemWidth();

		ImGuiUtils::ResetColumns();
		ImGui::PopID();
	}

	void PropertiesPanel::DrawModelProperties()
	{
		if (std::find(s_ModelExtensions.begin(), s_ModelExtensions.end(), m_AssetPath.extension().string()) != s_ModelExtensions.end())
		{
			std::ifstream modelFile(m_AssetPath.string() + ".model");

			static ModelImportSettings settings = { true, true, true, true, false, false, false, m_AssetPath.filename().string() };
			static bool normals = false, tangentSpace = false;
			static bool triangulate = false, joinVertices = false;
			// Prompt for importing
			ImGuiUtils::BoldText(m_AssetPath.filename().string() + " import settings");
			if (!modelFile.good())
				ImGui::TextWrapped("The selected object hasn't been imported. Customize the settings and hit \"Import\" to use the generated .model file.");
			ImGuiUtils::Separator();

			ImGuiUtils::StartColumns(2, { 200, (uint32_t)ImGui::GetContentRegionAvail().x - 200 });

			// Normals and tangent space
			ImGui::Text("Generate normals");
			ImGuiUtils::NextColumn();
			ImGui::Checkbox("##gennormals", &settings.Normals);
			ImGuiUtils::NextColumn();

			ImGui::Text("Generate tangent space");
			ImGuiUtils::NextColumn();
			ImGui::Checkbox("##gentangentspace", &settings.TangentSpace);
			ImGuiUtils::NextColumn();

			ImGuiUtils::Separator();

			// Triangulation
			ImGui::Text("Triangulate");
			ImGuiUtils::NextColumn();
			ImGui::Checkbox("##triangulate", &settings.Triangulate);
			ImGuiUtils::NextColumn();

			ImGui::Text("Join identical vertices");
			ImGuiUtils::NextColumn();
			ImGui::Checkbox("##joinvertices", &settings.JoinVertices);
			ImGuiUtils::NextColumn();

			ImGuiUtils::Separator();

			// Optimizations
			ImGui::Text("Optimize meshes");
			ImGuiUtils::NextColumn();
			ImGui::Checkbox("##optimizemeshes", &settings.OptimizeMeshes);
			ImGuiUtils::NextColumn();

			ImGui::Text("Optimize scene");
			ImGuiUtils::NextColumn();
			ImGui::Checkbox("##optimizescene", &settings.OptimizeScene);
			ImGuiUtils::NextColumn();

			ImGuiUtils::Separator();

			ImGui::Text("Imported name");
			ImGuiUtils::NextColumn();
			char tmpName[1024];
			memset(tmpName, 0, 1024);
			memcpy(tmpName, settings.ImportedName.c_str(), settings.ImportedName.length());
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputText("##importedname", tmpName, 1024, 0, 0, (void*)m_AssetPath.string().c_str()))
				settings.ImportedName = tmpName;
			ImGui::PopItemWidth();
			ImGuiUtils::NextColumn();

			ImGuiUtils::ResetColumns();

			if (ImGui::Button("Import", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
			{
				if (settings.ImportedName == "")
					settings.ImportedName = "Unnamed model";
				modelFile.close();
				ModelImporter::ImportModel(m_AssetPath.string(), settings);
				m_AssetPath = "";
				settings = {};
			}
		}
	}

	void PropertiesPanel::DrawPhysicsMaterial2DProperties()
	{
		Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(m_AssetPath.string());
		static PhysicsMaterial2DConfig config;
		if (m_AssetPath.compare(m_PrevAssetPath) != 0)
		{
			config.Density = material->GetDensity();
			config.Friction = material->GetFriction();
			config.Restitution = material->GetRestitution();
			config.RestitutionThreshold = material->GetRestitutionThreshold();
		}
		std::ifstream metaFile(material->GetPath());
		std::stringstream strStream;

		// PhysMat2D parameters
		ImGuiUtils::StartColumns(2, { 150, 200 });
		ImGuiUtils::DragFloat("Density", &config.Density, 0.1f, 0.0f, 1.0f);
		ImGuiUtils::DragFloat("Friction", &config.Friction, 0.1f, 0.0f, 1.0f);
		ImGuiUtils::DragFloat("Restitution", &config.Restitution, 0.3f, 0.0f, 100000.0f);
		ImGuiUtils::DragFloat("Restitution threshold", &config.RestitutionThreshold, 0.3f, 0.0f, 100000.0f);
		ImGuiUtils::ResetColumns();

		// Update settings
		if (ImGui::Button("Save settings", {ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f}))
		{
			PhysicsMaterial2D::SaveSettings(m_AssetPath.string(), config);
			material->SetConfig(config);
			material->Reload();
		}
	}

	void PropertiesPanel::DrawPhysicsMaterial3DProperties()
	{
		Ref<PhysicsMaterial3D> material = AssetManager::Request<PhysicsMaterial3D>(m_AssetPath.string());
		static PhysicsMaterial3DConfig config;
		if (m_AssetPath.compare(m_PrevAssetPath) != 0)
		{
			config.Friction = material->GetFriction();
			config.Restitution = material->GetRestitution();
		}
		std::ifstream metaFile(material->GetPath());
		std::stringstream strStream;

		// PhysMat2D parameters
		ImGuiUtils::StartColumns(2, { 150, 200 });
		ImGuiUtils::DragFloat("Friction", &config.Friction, 0.1f, 0.0f, 1.0f);
		ImGuiUtils::DragFloat("Restitution", &config.Restitution, 0.3f, 0.0f, 100000.0f);
		ImGuiUtils::ResetColumns();

		// Update settings
		if (ImGui::Button("Save settings", {ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f}))
		{
			PhysicsMaterial3D::SaveSettings(m_AssetPath.string(), config);
			material->SetConfig(config);
			material->Reload();
		}
	}

	void PropertiesPanel::DrawTextureProperties()
	{
		Ref<Texture2D> texture = AssetManager::Request<Texture2D>(m_AssetPath.string());
		// Reset texture parameters 
		static Texture2DConfig texParams;
		if (m_AssetPath.compare(m_PrevAssetPath) != 0)
		{
			texParams.Filtering = texture->GetFilteringMode();
			texParams.WrapMode = texture->GetWrapMode();
			texParams.ID = texture->GetID();
		}

		std::ifstream metaFile(texture->GetPath() + ".meta");
		std::stringstream strStream;

		std::string currFilterStdStr = Tex2DParamToString(texParams.Filtering);
		std::string currWrapStdStr = Tex2DParamToString(texParams.WrapMode);
		const char* currFilterString = currFilterStdStr.c_str();
		const char* currWrapString = currWrapStdStr.c_str();

		const char* filterTypes[] = { "Linear", "Point" };
		const char* newFilterType = nullptr;
		const char* wrapTypes[] = { "Clamp", "Repeat" };
		const char* newWrapMode = nullptr;

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		ImGuiUtils::StartColumns(2, { 100, 200 });
		if (ImGuiUtils::Combo("Filtering mode", filterTypes, 2, &currFilterString, &newFilterType))
			texParams.Filtering = StringToTex2DParam(newFilterType);

		ImGuiUtils::StartColumns(2, { 100, 200 });
		if (ImGuiUtils::Combo("Wrap mode", wrapTypes, 2, &currWrapString, &newWrapMode))
			texParams.WrapMode = StringToTex2DParam(newWrapMode);
		ImGuiUtils::ResetColumns();

		if (ImGui::TreeNodeEx("Texture preview", treeNodeFlags))
		{
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			ImVec2 textureSize;

			textureSize = { windowSize.x, windowSize.x / texture->GetAspectRatio() };
			ImGui::Image((ImTextureID)texture->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
			ImGui::TreePop();
		}

		// Update settings
		if (ImGui::Button("Save settings", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
		{
			Texture2D::SaveSettings(texParams, texture->GetPath());
			texture->Reload();
		}
	}

	void PropertiesPanel::DrawShaderProperties()
	{
		Ref<Shader> shader = AssetManager::Request<Shader>(m_AssetPath.string());
		std::vector<ShaderUniform> uniforms = shader->GetUniforms();

		ImGui::Text("Shader uniforms");

		// Show uniforms
		ImGuiUtils::StartColumns(2, { 150, 200 });
		for (auto& uniform : uniforms)
		{
			ImGui::PushID(uniform.Name.c_str());
			ImGui::Text(uniform.Name.c_str());
			ImGui::NextColumn();
			ImGui::Text(ShaderDataTypeToString(uniform.Type).c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGuiUtils::ResetColumns();
	}

	void PropertiesPanel::DrawMaterialProperties()
	{
		std::string metaPath;
		if (m_MetaPath.compare("") == 0)
			metaPath = m_AssetPath.string() + ".meta";
		else
			metaPath = m_MetaPath.string();
		static MaterialConfig config;
		Ref<Material> material = AssetManager::Request<Material>(m_AssetPath.string(), metaPath);
		if (material == nullptr)
		{
			std::ifstream metaFile(metaPath);
			std::stringstream ss;
			ss << metaFile.rdbuf();
			YAML::Node metaNode = YAML::Load(ss.str());
			material = AssetManager::Request<Material>(metaNode["ID"].as<uint64_t>());
		}

		if (m_AssetPath.compare(m_PrevAssetPath) != 0)
		{
			config.Name = material->GetName();
			config.Shader = material->GetShader();
			config.Uniforms = material->GetUniforms();
			config.ID = material->GetID();
		}

		Ref<Shader> shader = AssetManager::Request<Shader>(config.Shader);
		material->SetShader(shader);
		material->SetUniforms(config.Uniforms);

		// Shader selection combobox
		const char** shaders;
		const char* currShader = shader == nullptr ? "None" : shader->GetName().c_str();
		const char* ret = nullptr;

		// Get all shaders
		std::filesystem::path shaderFolder("assets\\shaders");
		std::vector<std::string> shaderStrings = CppUtils::FileSystem::GetAllFilesWithExtension(".glsl", "assets\\shaders");

		// Convert the strings to const char*s
		shaders = new const char* [shaderStrings.size()];
		for (uint32_t i = 0; i < shaderStrings.size(); i++)
			shaders[i] = shaderStrings[i].c_str();

		// Draw the combobox to choose the shader
		if (ImGuiUtils::Combo("Shader", shaders, shaderStrings.size(), &currShader, &ret))
		{
			Ref<Shader> loadedShader = AssetManager::Request<Shader>(ret);
			std::vector<ShaderUniform> uniforms = loadedShader->GetUniforms();

			config.Shader = loadedShader->GetID();
			config.Uniforms.clear();
			for (auto& uniform : uniforms)
				config.Uniforms[uniform.Name] = uniform;
		}
		delete[] shaders;

		// Get uniforms that are set by the engine so the user can't edit them
		std::vector<std::string> defaultUniforms = Material::GetDefaultUniforms();
		// Get texture uniforms so we can render all the attributes together
		std::vector<std::string> samplers;
		for (auto& uniform : config.Uniforms)
			if (uniform.second.Type == ShaderDataType::Sampler2D && CppUtils::String::EndsWith(uniform.second.Name, ".Sampler"))
				samplers.push_back(uniform.second.Name.substr(0, uniform.second.Name.find_last_of(".")));
				
		if (ImGui::TreeNodeEx("mat_shader_props", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding, "Properties"))
		{
			// Draw Material properties
			for (auto& uniform : config.Uniforms)
			{
				bool draw = true;
				bool textureStruct = false;
				std::string textureName;

				// Don't draw uniforms set by the engine, the user has no control over them
				for (auto& defaultUniform : defaultUniforms)
				{
					if (uniform.second.Name.find(defaultUniform) != std::string::npos)
					{
						draw = false;
						break;
					}
				}

				// Don't draw attributes belonging to the Texture structure, it will be taken care of in the Sampler2D case
				for (auto& texture : samplers)
				{
					if (CppUtils::String::StartsWith(uniform.second.Name, texture))
					{
						if (uniform.second.Type == ShaderDataType::Sampler2D)
						{
							textureStruct = true;
							textureName = texture;
						}
						else
							draw = false;
						break;
					}
				}

				if (draw)
				{
					ImGui::PushID(uniform.second.Name.c_str());
					switch (uniform.second.Type)
					{
					case ShaderDataType::Bool:
						if (ImGui::Checkbox(uniform.second.Name.c_str(), &std::get<bool>(uniform.second.Data)))
							config.Uniforms[uniform.second.Name].Data = uniform.second.Data;
						break;

					case ShaderDataType::Float:
					{
						float value = std::get<float>(uniform.second.Data);
						if (ImGuiUtils::DragFloat(uniform.second.Name, &value, 0.15f))
							config.Uniforms[uniform.second.Name].Data = value;
						break;
					}
					case ShaderDataType::Float2:
					{
						glm::vec2 vec = std::get<glm::vec2>(uniform.second.Data);
						float a = vec.x, b = vec.y;
						ImGuiUtils::RGBVec2(uniform.second.Name.c_str(), { "A","B" }, { &a, &b });
						config.Uniforms[uniform.second.Name].Data = glm::vec2(a, b);

						break;
					}
					case ShaderDataType::Float3:
					{
						glm::vec3 vec = std::get<glm::vec3>(uniform.second.Data);
						float a = vec.x, b = vec.y, c = vec.z;
						ImGuiUtils::RGBVec3(uniform.second.Name.c_str(), { "A","B","C" }, { &a, &b, &c });
						config.Uniforms[uniform.second.Name].Data = glm::vec3(a, b, c);

						break;
					}
					case ShaderDataType::Float4:
					{
						glm::vec4 vec = std::get<glm::vec4>(uniform.second.Data);
						float a = vec.x, b = vec.y, c = vec.z, d = vec.w;
						ImGuiUtils::RGBVec4(uniform.second.Name.c_str(), { "A","B","C","D" }, { &a, &b, &c, &d });
						config.Uniforms[uniform.second.Name].Data = glm::vec4(a, b, c, d);

						break;
					}
					case ShaderDataType::Sampler2D:
					{
						if (!textureStruct)
						{
							ImGuiUtils::StartColumns(2, { 90, (uint32_t)ImGui::GetContentRegionAvail().x - 90 });
							// Load the texture: if it doesn't exist, just use a white default texture
							uint32_t rendererID;
							Ref<Texture2D> currTexture = AssetManager::Request<Texture2D>(std::get<UUID>(uniform.second.Data));
							if (currTexture == nullptr)
								rendererID = EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID();
							else
								rendererID = currTexture->GetRendererID();

							// Texture preview button
							std::stringstream ss;
							ss << "Texture" << rendererID << uniform.second.Name;
							Ref<Texture2D> newTexture = ImGuiUtils::ImageDragDestination<Texture2D>(rendererID, { 80, 80 }, ss.str().c_str());
							if (newTexture != nullptr)
								config.Uniforms[uniform.second.Name].Data = newTexture->GetID();

							ImGui::NextColumn();

							// Texture title
							ImGui::Text(("Texture " + uniform.second.Name).c_str());

							ImGuiUtils::ResetColumns();
						}
						else
						{
							ImGuiUtils::StartColumns(2, { 110, (uint32_t)ImGui::GetContentRegionAvail().x - 100 });
							// Load the texture: if it doesn't exist, just use a white default texture
							uint32_t rendererID;
							Ref<Texture2D> currTexture = AssetManager::Request<Texture2D>(std::get<UUID>(uniform.second.Data));
							if (currTexture == nullptr)
								rendererID = EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID();
							else
								rendererID = currTexture->GetRendererID();

							// Texture preview button
							std::stringstream ss;
							ss << "Texture" << rendererID << uniform.second.Name;
							Ref<Texture2D> newTexture = ImGuiUtils::ImageDragDestination<Texture2D>(rendererID, { 90, 90 }, ss.str().c_str());
							if (newTexture != nullptr)
								config.Uniforms[uniform.second.Name].Data = newTexture->GetID();

							ImGui::NextColumn();

							ImGui::BeginChild("Next data", {ImGui::GetContentRegionAvail().x, 100});

							// Texture title and use button
							if (config.Uniforms.find(textureName + ".Use") == config.Uniforms.end())
								config.Uniforms[textureName + ".Use"].Data = false;
							bool val = std::get<bool>(config.Uniforms[textureName + ".Use"].Data);
							if (ImGui::Checkbox(("Texture " + uniform.second.Name).c_str(), &val))
								config.Uniforms[textureName + ".Use"].Data = val;

							
							// Tiling
							if (config.Uniforms.find(textureName + ".Tiling") == config.Uniforms.end())
								config.Uniforms[textureName + ".Tiling"].Data = glm::vec2(1, 1);
							glm::vec2 tiling = std::get<glm::vec2>(config.Uniforms[textureName + ".Tiling"].Data);
							ImGuiUtils::RGBVec2("Tiling", { "X","Y" }, { &tiling.x, &tiling.y }, 0.0f, 80);
							config.Uniforms[textureName + ".Tiling"].Data = tiling;


							// Offset
							if (config.Uniforms.find(textureName + ".Offset") == config.Uniforms.end())
								config.Uniforms[textureName + ".Offset"].Data = glm::vec2(0, 0);
							glm::vec2 offset = std::get<glm::vec2>(config.Uniforms[textureName + ".Offset"].Data);
							ImGuiUtils::RGBVec2("Offset", { "X","Y" }, { &offset.x, &offset.y }, 0.0f, 80);
							config.Uniforms[textureName + ".Offset"].Data = offset;

							// Intensity
							ImGuiUtils::DragFloat("Intensity", &std::get<float>(config.Uniforms[textureName + ".Intensity"].Data), 0.1f, -1000, 1000, 80);
							
							ImGui::EndChild();
							ImGuiUtils::ResetColumns();
						}
						
						break;
					}
					case ShaderDataType::None:
						break;
					default:
						break;
					}

					ImGuiUtils::VerticalSpace(5);
					ImGui::PopID();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("Save settings", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
		{
			Material::SaveSettings(material->GetPath(), config);
			material->Reload();
		}
	}

	void PropertiesPanel::DrawSkyboxProperties()
	{
		// Load necessary resources
		Ref<Skybox> skybox = AssetManager::Request<Skybox>(m_AssetPath.string());

		// Load skybox config or keep the current one if the selected asset is the same
		static SkyboxConfig skyboxConfig;
		if (m_AssetPath.compare(m_PrevAssetPath) != 0)
		{
			skyboxConfig.Textures[SkyboxTexture::Bottom] = skybox->GetTexture(SkyboxTexture::Bottom);
			skyboxConfig.Textures[SkyboxTexture::Down] = skybox->GetTexture(SkyboxTexture::Down);
			skyboxConfig.Textures[SkyboxTexture::Up] = skybox->GetTexture(SkyboxTexture::Up);
			skyboxConfig.Textures[SkyboxTexture::Left] = skybox->GetTexture(SkyboxTexture::Left);
			skyboxConfig.Textures[SkyboxTexture::Right] = skybox->GetTexture(SkyboxTexture::Right);
			skyboxConfig.Textures[SkyboxTexture::Front] = skybox->GetTexture(SkyboxTexture::Front);
			skyboxConfig.Material = skybox->GetMaterial();
			skyboxConfig.ID = skybox->GetID();
		}
		Ref<Material> material = AssetManager::Request<Material>(skyboxConfig.Material);

		// Material
		UUID currentMaterial = ImGuiUtils::DragDestination("Material", ".mat", material == nullptr ? 0 : material->GetID());
		if (currentMaterial != 0)
			skyboxConfig.Material = currentMaterial;

		ImGuiUtils::ResetColumns();

		// Drag / drop textures
		SkyboxTexture dirs[6] = { SkyboxTexture::Front, SkyboxTexture::Bottom, SkyboxTexture::Left,
			SkyboxTexture::Right, SkyboxTexture::Up, SkyboxTexture::Down };
		const char* dirStrings[6] = { "Front", "Bottom", "Left", "Right", "Up", "Down" };

		ImGui::Text("Textures");

		ImGuiUtils::StartColumns(4, { 100, 100, 100, 100 });
		for (uint32_t i = 0; i < 6; i++)
		{
			Ref<Texture2D> preview = AssetManager::Request<Texture2D>(skyboxConfig.Textures[dirs[i]]);
			if (preview == nullptr)
				preview = EditorCache::Textures().Get("assets\\textures\\empty_texture.png");
			Ref<Texture2D> newTexture = ImGuiUtils::ImageDragDestination<Texture2D>(preview->GetRendererID(), { 80, 80 });

			// User loaded new texture
			if (newTexture != nullptr && newTexture->GetRendererID() != preview->GetRendererID())
				skyboxConfig.Textures[dirs[i]] = newTexture->GetID();
			ImGui::NextColumn();

			ImGui::Text(std::string(std::string(dirStrings[i]) + " texture").c_str());
			ImGui::NextColumn();
		}
		ImGuiUtils::ResetColumns();

		// Apply settings button
		if (ImGui::Button("Apply settings"))
		{
			skybox->SaveSettings(skyboxConfig, m_AssetPath.string());
			skybox->Reload();
		}
	}

	void PropertiesPanel::DrawPostProcessingStackProperties()
	{
		Ref<PostProcessingStack> stack = AssetManager::Request<PostProcessingStack>(m_AssetPath.string());
		static PostProcessingStackConfig config = {0};

		if (config.ID == 0)
			config = stack->GetConfig(m_AssetPath.string());

		if (stack->GetVolumes().size() == 0)
			ImGui::Text("Click on \"Add Volume\" to select a post-processing effect");

		if (ImGui::Button("Add volume", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
			ImGui::OpenPopup("AddVolumeMenu");			

		if (ImGui::BeginPopup("AddVolumeMenu"))
		{
			bool addedVolume = false;
			std::string volumeName;
			PostProcessingEffect effectType;
			Ref<Shader> shader;

			if (ImGui::MenuItem("Anti Aliasing"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\antialiasing.glsl");
				volumeName = "Anti Aliasing";
				effectType = PostProcessingEffect::AntiAliasing;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("Bloom"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\bloom.glsl");
				volumeName = "Bloom";
				effectType = PostProcessingEffect::Bloom;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("Blur"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\blur.glsl");
				volumeName = "Blur";
				effectType = PostProcessingEffect::Blur;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("Depth of field"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\depthoffield.glsl");
				volumeName = "Depth of field";
				effectType = PostProcessingEffect::DepthOfField;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("White balance"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\whitebalance.glsl");
				volumeName = "White balance";
				effectType = PostProcessingEffect::WhiteBalance;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("Fog"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\fog.glsl");
				volumeName = "Fog";
				effectType = PostProcessingEffect::Fog;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("Ambient Occlusion"))
			{
				shader = AssetManager::Request<Shader>("assets\\shaders\\PostProcessing\\ambientocclusion.glsl");
				volumeName = "Ambient Occlusion";
				effectType = PostProcessingEffect::AmbientOcclusion;
				addedVolume = true;
			}
			else if (ImGui::MenuItem("Custom"))
			{
				stack->PushVolume(PostProcessingVolume(0));
				config.Volumes.push_back(PostProcessingVolume(0));
			}

			if (addedVolume)
			{
				PostProcessingVolume toAdd;
				toAdd.Enabled = true;
				toAdd.Name = volumeName;
				toAdd.RuntimeShader = shader;
				if (shader != nullptr)
				{
					toAdd.ShaderID = shader->GetID();
					for (auto& uniform : shader->GetUniforms())
						toAdd.Properties[uniform.Name] = uniform;
				}
				toAdd.Type = effectType;
				
				stack->PushVolume(toAdd);
				config.Volumes.push_back(toAdd);
			}
			ImGui::EndPopup();
		}

		uint32_t i = 0;
		int toDelete = -1;

		for (auto& volume : stack->GetVolumes())
		{
			std::stringstream ss;
			ss << "##" <<volume.Name << i;
			if (ImGui::Checkbox(ss.str().c_str(), &volume.Enabled))
				config.Volumes[i].Enabled = volume.Enabled;

			ImGui::SameLine();

			static char buffer[256];
			ss.str();
			ss << "###" << i;
			memset(buffer, 0, 256);
			memcpy(buffer, volume.Name.c_str(), volume.Name.length());
			ImGui::InputText(ss.str().c_str(), buffer, 256);
			volume.Name = std::string(buffer);
			config.Volumes[i].Name = volume.Name;

			if (volume.Type != PostProcessingEffect::Custom)
			{
				UUID shader = ImGuiUtils::DragDestination("Shader", ".glsl", volume.ShaderID);
				if (shader != 0)
				{
					std::vector<ShaderUniform> uniforms;
					volume.RuntimeShader = AssetManager::Request<Shader>(shader);
					volume.ShaderID = volume.RuntimeShader->GetID();
					uniforms = volume.RuntimeShader->GetUniforms();

					volume.Properties.clear();

					for (auto& uniform : uniforms)
						volume.Properties[uniform.Name] = uniform;
					config.Volumes[i].ShaderID = volume.ShaderID;
					config.Volumes[i].Properties = volume.Properties;
				}
			}

			for (auto& prop : volume.Properties)
			{
				if (prop.second.Name.compare("u_Texture") == 0)
					continue;
				switch (prop.second.Type)
				{
				case ShaderDataType::Float:
					ImGuiUtils::DragFloat(prop.second.Name, &std::get<float>(prop.second.Data), 0.02f);
					config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					break;
				case ShaderDataType::Float2:
				{
					glm::vec2& vec = std::get<glm::vec2>(prop.second.Data);
					ImGuiUtils::RGBVec2(prop.second.Name.c_str(), { "X", "Y" }, { &vec.x, &vec.y });
					config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					break;
				}
				case ShaderDataType::Float3:
				{
					glm::vec3& vec = std::get<glm::vec3>(prop.second.Data);
					ImGuiUtils::RGBVec3(prop.second.Name.c_str(), { "X", "Y", "Z" }, { &vec.x, &vec.y, &vec.z });
					config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					break;
				}
				case ShaderDataType::Float4:
				{
					glm::vec4& vec = std::get<glm::vec4>(prop.second.Data);
					ImGuiUtils::RGBVec4(prop.second.Name.c_str(), { "X", "Y", "Z", "W" }, { &vec.x, &vec.y, &vec.z, &vec.w });
					config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					break;
				}
				case ShaderDataType::Bool:
					ImGui::Checkbox(prop.second.Name.c_str(), &std::get<bool>(prop.second.Data));
					config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					break;
				case ShaderDataType::Sampler2D:
				{
					ImGuiUtils::StartColumns(2, { 90, (uint32_t)ImGui::GetContentRegionAvail().x - 90 });
					// Load the texture: if it doesn't exist, just use a white default texture
					uint32_t rendererID;
					Ref<Texture2D> currTexture = AssetManager::Request<Texture2D>(std::get<UUID>(prop.second.Data));
					if (currTexture == nullptr)
						rendererID = EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID();
					else
						rendererID = currTexture->GetRendererID();

					// Texture preview button
					std::stringstream ss;
					ss << "Texture" << rendererID << prop.second.Name;
					Ref<Texture2D> newTexture = ImGuiUtils::ImageDragDestination<Texture2D>(rendererID, { 80, 80 }, ss.str().c_str());
					if (newTexture != nullptr)
					{
						prop.second.Data = newTexture->GetID();
						config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					}

					ImGui::NextColumn();

					// Texture title
					ImGui::Text(("Texture " + prop.second.Name).c_str());

					ImGuiUtils::ResetColumns();
					break;
				}
				case ShaderDataType::Int:
					ImGuiUtils::DragInt(prop.second.Name, &std::get<int>(prop.second.Data), 0.1f);
					config.Volumes[i].Properties[prop.second.Name].Data = prop.second.Data;
					break;
				}
			}

			if (ImGui::Button("Delete volume", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
				toDelete = i;
			i++;
			ImGuiUtils::Separator();
		}

		if (toDelete >= 0)
		{
			stack->RemoveVolume(toDelete);
			config.Volumes.erase(config.Volumes.begin() + toDelete);
		}

		if (ImGui::Button("Apply settings", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
			stack->SaveSettings(m_AssetPath.string(), config);
	}

	void PropertiesPanel::SetAsset(std::filesystem::path path, std::filesystem::path metaPath, AssetType assetType)
	{
		if (std::filesystem::is_directory(path))
			return;
		m_PrevAssetPath = m_AssetPath;
		m_AssetPath = path;
		m_MetaPath = metaPath;
		m_AssetType = assetType;

		if (std::find(s_ModelExtensions.begin(), s_ModelExtensions.end(), m_AssetPath.extension().string()) != s_ModelExtensions.end())
			m_AssetType = AssetType::Model;
		else if (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg" || path.extension() == ".tga")
			m_AssetType = AssetType::Texture2D;
		else if (path.extension() == ".physmat2d")
			m_AssetType = AssetType::PhysicsMaterial2D;
		else if (path.extension() == ".physmat3d")
			m_AssetType = AssetType::PhysicsMaterial3D;
		else if (path.extension() == ".glsl")
			m_AssetType = AssetType::Shader;
		else if (path.extension() == ".mat")
			m_AssetType = AssetType::Material;
		else if (path.extension() == ".mesh")
			m_AssetType = AssetType::Mesh;
		else if (path.extension() == ".skybox")
			m_AssetType = AssetType::Skybox;
		else if (path.extension() == ".postps")
			m_AssetType = AssetType::PostProcessingStack;
	}
}