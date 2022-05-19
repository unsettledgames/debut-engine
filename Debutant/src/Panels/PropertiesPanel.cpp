#include "PropertiesPanel.h"
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Renderer/Texture.h>
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <imgui_internal.h>
#include <Debut/ImGui/ImGuiUtils.h>

/**
	TODO:
	- Polish: reset texture paramters so they're coherent if the user doesn't save the results
*/

namespace Debutant
{
	static void GenerateTextureData(const Texture2DConfig& parameters, std::string& path)
	{
		YAML::Emitter emitter;

		emitter << YAML::BeginDoc << YAML::BeginMap;

		emitter << YAML::Key << "Asset" << YAML::Value << "Texture2D";
		emitter << YAML::Key << "ID" << YAML::Value << parameters.ID;
		emitter << YAML::Key << "Filtering" << YAML::Value << Tex2DParamToString(parameters.Filtering);
		emitter << YAML::Key << "WrapMode" << YAML::Value << Tex2DParamToString(parameters.WrapMode);

		emitter << YAML::EndMap << YAML::EndDoc;

		std::ofstream outFile(path + ".meta");
		outFile << emitter.c_str();
	}

	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::Begin("Properties");

		if (m_AssetPath != "")
		{
			// Put extensions as static members of Texture2D and PhysicsMaterial2D?
			if (m_AssetPath.extension().string() == ".png")
			{
				DrawTextureProperties();
			}
			else if (m_AssetPath.extension().string() == ".physmat2d")
			{
				DrawPhysicsMaterial2DProperties();
			}
		}

		ImGui::End();
	}

	void PropertiesPanel::DrawPhysicsMaterial2DProperties()
	{
		Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(m_AssetPath.string());
		std::ifstream metaFile(material->GetPath());
		std::stringstream strStream;

		Texture2DConfig texParams = { Texture2DParameter::FILTERING_LINEAR, Texture2DParameter::WRAP_CLAMP };

		if (metaFile.good())
		{
			strStream << metaFile.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			texParams.Filtering = StringToTex2DParam(in["Filtering"].as<std::string>());
			texParams.WrapMode = StringToTex2DParam(in["WrapMode"].as<std::string>());
		}

		float density = material->GetDensity(), friction = material->GetFriction(),
			restitution = material->GetRestitution(), restitutionThreshold = material->GetRestitutionThreshold();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::LabelText("##importtitle", (m_AssetPath.filename().string() + " import settings").c_str(), 50);
		ImGui::PopFont();

		// PhysMat2D parameters
		ImGuiUtils::StartColumns(2, { 150, 200 });
			ImGuiUtils::DragFloat("Density", &density, 0.1f, 0.0f, 1.0f);
			ImGuiUtils::DragFloat("Friction", &friction, 0.1f, 0.0f, 1.0f);
			ImGuiUtils::DragFloat("Restitution", &restitution, 0.3f, 0.0f, 100000.0f);
			ImGuiUtils::DragFloat("Restitution threshold", &restitutionThreshold, 0.3f, 0.0f, 100000.0f);
		ImGuiUtils::ResetColumns();

		// When settings are saved, a meta file containing the data is generated for that texture
		if (ImGui::Button("Save settings"))
		{
			PhysicsMaterial2DConfig config;
			config.Density = density;
			config.Friction = friction;
			config.Restitution = restitution;
			config.RestitutionThreshold = restitutionThreshold;

			//PhysicsMaterial2D::SaveSettings(config);
			material->SetConfig(config);
		}

		ImGui::PopStyleVar();
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

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::LabelText("##importtitle", (m_AssetPath.filename().string() + " import settings").c_str(), 50);
		ImGui::PopFont();

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

		// When settings are saved, a meta file containing the data is generated for that texture
		if (ImGui::Button("Save settings"))
		{
			texParams.ID = texture->GetID();

			GenerateTextureData(texParams, texture->GetPath());
			texture->Reload();
		}

		ImGui::PopStyleVar();
	}

	void PropertiesPanel::SetAsset(std::filesystem::path path)
	{
		m_AssetPath = path;
	}
}