#include "PropertiesPanel.h"
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Texture.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <stack>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <imgui_internal.h>
#include <Debut/ImGui/ImGuiUtils.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Shader.h>

/**
	TODO:
	- Polish: reset texture paramters so they're coherent if the user doesn't save the results
*/

namespace Debut
{
	static std::vector<std::string> s_SupportedExtensions = { ".png", ".physmat2d", ".glsl", ".mat"};

	static int SetFileName(ImGuiInputTextCallbackData* data)
	{
		int sas = 2;
		return 0;
	}

	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::Begin("Properties");
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });

		if (m_AssetPath != "")
		{
			if (std::find(s_SupportedExtensions.begin(), s_SupportedExtensions.end(), m_AssetPath.extension().string()) != s_SupportedExtensions.end())
			{
				DrawName();
			}

			// Put extensions as static members of Texture2D and PhysicsMaterial2D?
			if (m_AssetPath.extension().string() == ".png")
			{
				DrawTextureProperties();
			}
			else if (m_AssetPath.extension().string() == ".physmat2d")
			{
				DrawPhysicsMaterial2DProperties();
			}
			else if (m_AssetPath.extension().string() == ".glsl")
			{
				DrawShaderProperties();
			}
			else if (m_AssetPath.extension().string() == ".mat")
			{
				DrawMaterialProperties();
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
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
			// Set name and edit it in the asset manager
		}

		ImGui::PopItemWidth();

		ImGuiUtils::ResetColumns();
		ImGui::PopID();
		
	}

	void PropertiesPanel::DrawPhysicsMaterial2DProperties()
	{
		Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(m_AssetPath.string());
		std::ifstream metaFile(material->GetPath());
		std::stringstream strStream;

		// PhysMat2D parameters
		ImGuiUtils::StartColumns(2, { 150, 200 });
			ImGuiUtils::DragFloat("Density", &material->m_Density, 0.1f, 0.0f, 1.0f);
			ImGuiUtils::DragFloat("Friction", &material->m_Friction, 0.1f, 0.0f, 1.0f);
			ImGuiUtils::DragFloat("Restitution", &material->m_Restitution, 0.3f, 0.0f, 100000.0f);
			ImGuiUtils::DragFloat("Restitution threshold", &material->m_RestitutionThreshold, 0.3f, 0.0f, 100000.0f);
		ImGuiUtils::ResetColumns();

		// Update settings
		if (ImGui::Button("Save settings"))
		{
			PhysicsMaterial2DConfig config;
			config.Density = material->m_Density;
			config.Friction = material->m_Friction;
			config.Restitution = material->m_Restitution;
			config.RestitutionThreshold = material->m_RestitutionThreshold;

			PhysicsMaterial2D::SaveSettings(m_AssetPath.string(), config);
			material->SetConfig(config);
		}
	}

	void PropertiesPanel::DrawTextureProperties()
	{
		Ref<Texture2D> texture = AssetManager::Request<Texture2D>(m_AssetPath.string());
		std::ifstream metaFile(texture->GetPath() + ".meta");
		std::stringstream strStream;

		Texture2DConfig texParams = { Texture2DParameter::FILTERING_LINEAR, Texture2DParameter::WRAP_CLAMP };

		Texture2DParameter filter = texture->GetFilteringMode();
		Texture2DParameter wrapMode = texture->GetWrapMode();

		std::string currFilterStdStr = Tex2DParamToString(filter);
		std::string currWrapStdStr = Tex2DParamToString(wrapMode);
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
		{
			texParams.Filtering = StringToTex2DParam(newFilterType);
			texture->SetFilteringMode(texParams.Filtering);
		}

		ImGuiUtils::StartColumns(2, { 100, 200 });
		if (ImGuiUtils::Combo("Wrap mode", wrapTypes, 2, &currWrapString, &newWrapMode))
		{
			texParams.WrapMode = StringToTex2DParam(newWrapMode);
			texture->SetWrapMode(texParams.WrapMode);
		}
		
		ImGui::Columns(1);

		if (ImGui::TreeNodeEx("Texture preview", treeNodeFlags))
		{
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			ImVec2 textureSize;

			textureSize = { windowSize.x, windowSize.x / texture->GetAspectRatio() };
			ImGui::Image((ImTextureID)texture->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
			ImGui::TreePop();
		}

		// Update settings
		if (ImGui::Button("Save settings"))
		{
			texParams.ID = texture->GetID();
			texParams.WrapMode = texture->GetWrapMode();
			texParams.Filtering = texture->GetFilteringMode();

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
		MaterialConfig finalConfig;

		Ref<Material> material = AssetManager::Request<Material>(m_AssetPath.string());
		Ref<Shader> shader = AssetManager::Request<Shader>(material->GetShader());

		// Shader selection combobox
		std::vector<std::string> shaderStrings;
		const char** shaders;
		std::filesystem::path shaderFolder("assets\\shaders");

		const char* currShader = shader == nullptr ? "None" : shader->GetName().c_str();
		const char* ret = nullptr;

		// BF visit to find all the shaders in the folder
		std::stack<std::filesystem::path> pathsToVisit;
		pathsToVisit.push(std::filesystem::path("assets\\shaders"));
		std::filesystem::path currPath;

		// TODO: cache paths
		while (pathsToVisit.size() > 0)
		{
			currPath = pathsToVisit.top();
			pathsToVisit.pop();

			auto& dirIt = std::filesystem::directory_iterator(currPath);

			for (auto entry : dirIt)
			{
				if (entry.is_directory())
					pathsToVisit.push(entry.path());
				else
				{
					std::string extension = entry.path().extension().string();
					if (extension == ".glsl" || extension == ".hlsl")
						shaderStrings.push_back(entry.path().string());
				}
			}
		}

		// Convert the strings to const char*s
		shaders = new const char* [shaderStrings.size()];
		for (uint32_t i = 0; i < shaderStrings.size(); i++)
			shaders[i] = shaderStrings[i].c_str();

		// Draw the combobox to choose the shader
		if (ImGuiUtils::Combo("Shader", shaders, shaderStrings.size(), &currShader, &ret))
		{
			Ref<Shader> loadedShader = AssetManager::Request<Shader>(ret);
			material->SetShader(loadedShader);
		}
		delete[] shaders;

		if (ImGui::TreeNodeEx("mat_shader_props", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding, "Properties"))
		{
			// Draw Material properties
			for (auto uniform : material->GetUniforms())
			{
				switch (uniform.Type)
				{
				case ShaderDataType::Float:
				{
					float value = uniform.Data.Float;
					if (ImGuiUtils::DragFloat(uniform.Name, &value, 0.15f))
						material->SetFloat(uniform.Name, value);
					break;
				}
				case ShaderDataType::Float2:
				{
					float a = uniform.Data.Vec2.x, b = uniform.Data.Vec2.y;
					ImGuiUtils::RGBVec2(uniform.Name.c_str(), { "A","B" }, { &a, &b });
					material->SetVec2(uniform.Name, { a, b });

					break;
				}
				case ShaderDataType::Float3:
				{
					float a = uniform.Data.Vec3.x, b = uniform.Data.Vec3.y, c = uniform.Data.Vec3.z;
					ImGuiUtils::RGBVec3(uniform.Name.c_str(), { "A","B","C" }, { &a, &b, &c });
					material->SetVec3(uniform.Name, { a, b, c });

					break;
				}
				case ShaderDataType::Float4:
				{
					float a = uniform.Data.Vec3.x, b = uniform.Data.Vec3.y, c = uniform.Data.Vec3.z, d = uniform.Data.Vec4.w;
					ImGuiUtils::RGBVec4(uniform.Name.c_str(), { "A","B","C","D" }, { &a, &b, &c, &d });
					material->SetVec4(uniform.Name, { a, b, c, d });

					break;
				}
				case ShaderDataType::Sampler2D:
				{
					// Load the texture: if it doesn't exist, just use a white default texture
					uint32_t rendererID;
					Ref<Texture2D> currTexture = AssetManager::Request<Texture2D>(uniform.Data.Texture);
					if (currTexture == nullptr)
						rendererID = EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID();
					else
						rendererID = currTexture->GetRendererID();

					// Texture title
					ImGuiUtils::BoldText("Texture " + uniform.Name);

					// Texture preview button
					Ref<Texture2D> newTexture = ImGuiUtils::ImageDragDestination<Texture2D>(rendererID, { 64, 64 });
					if (newTexture != nullptr)
						material->SetTexture(uniform.Name, newTexture);

					// TODO: Size and offset?
					break;
				}

				default:
					break;
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("Save settings"))
		{
			material->SaveSettings();
		}
		
	}

	void PropertiesPanel::SetAsset(std::filesystem::path path)
	{
		if (std::filesystem::is_directory(path))
			return;
		m_AssetPath = path;
	}
}