#include "PropertiesPanel.h"
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Renderer/Texture.h>
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>

/* Almost there
* - Remove distinction between mig / mag filtering, just use one
**/

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
			if (m_AssetPath.extension().string() == ".png")
			{
				DrawTextureProperties();
			}
		}

		ImGui::End();
	}

	void PropertiesPanel::DrawTextureProperties()
	{
		Ref<Texture2D> texture = AssetManager::RequestTexture(m_AssetPath.string());
		std::ifstream metaFile(texture->GetPath() + ".meta");
		std::stringstream strStream;

		Texture2DConfig texParams = { Texture2DParameter::FILTERING_LINEAR, Texture2DParameter::WRAP_CLAMP };

		if (metaFile.good())
		{
			strStream << metaFile.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			texParams.Filtering = StringToTex2DParam(in["Filtering"].as<std::string>());
			texParams.WrapMode = StringToTex2DParam(in["WrapMode"].as<std::string>());
		}
		
		Texture2DParameter filter = texture->GetFilteringMode();
		Texture2DParameter wrapMode = texture->GetWrapMode();

		std::string currFilterString = Tex2DParamToString(filter);
		std::string currWrapString = Tex2DParamToString(wrapMode);

		const char* filterTypes[] = { "Linear", "Point" };
		const char* wrapTypes[] = { "Clamp", "Repeat" };

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::LabelText("##importtitle", (m_AssetPath.filename().string() + " import settings").c_str(), 50);
		ImGui::PopFont();


		ImGui::Columns(2);
		// Min filtering 
		ImGui::Text("Filterig mode");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##filter", Tex2DParamToString(filter).c_str()))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = currFilterString == filterTypes[i];
				if (ImGui::Selectable(filterTypes[i], &isSelected))
				{
					currFilterString = filterTypes[i];
					texture->SetFilteringMode(StringToTex2DParam(std::string(currFilterString)));
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		// Wrap mode
		ImGui::NextColumn();
		ImGui::Text("Wrap mode");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##wrapmode", Tex2DParamToString(wrapMode).c_str()))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = currWrapString == wrapTypes[i];
				if (ImGui::Selectable(wrapTypes[i], &isSelected))
				{
					currWrapString = wrapTypes[i];
					texture->SetWrapMode(StringToTex2DParam(std::string(currWrapString)));
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
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
			Texture2DConfig config;
			config.Filtering = StringToTex2DParam(currFilterString);
			config.WrapMode = StringToTex2DParam(currWrapString);
			config.ID = texture->GetID();

			GenerateTextureData(config, texture->GetPath());
			texture->Reload();
		}

		ImGui::PopStyleVar();
	}

	void PropertiesPanel::SetAsset(std::filesystem::path path)
	{
		if (path.extension().string() == ".png")
			m_AssetPath = path;
	}
}