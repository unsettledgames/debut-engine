#include "PropertiesPanel.h"
#include "Utils/EditorCache.h"
#include <Debut/Renderer/Texture.h>
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>

/* Almost there
* - Update texture if it's loaded
**/

namespace Debutant
{
	static void GenerateTextureData(const Texture2DConfig& parameters, std::string& path)
	{
		YAML::Emitter emitter;

		emitter << YAML::BeginDoc << YAML::BeginMap;

		emitter << YAML::Key << "Asset" << YAML::Value << "Texture2D";
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
		Ref<Texture2D> texture = EditorCache::Textures().Get(m_AssetPath.string());
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
		
		Texture2DParameter magFilt = texture->GetMagFiltering();
		Texture2DParameter minFilt = texture->GetMinFiltering();
		Texture2DParameter wrapMode = texture->GetWrapMode();

		std::string currMagString = Tex2DParamToString(magFilt);
		std::string currMinString = Tex2DParamToString(magFilt);
		std::string currWrapString = Tex2DParamToString(wrapMode);

		const char* magTypes[] = { "Linear", "Point" };
		const char* minTypes[] = { "Linear", "Point" };
		const char* wrapTypes[] = { "Clamp", "Repeat" };

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::LabelText("##importtitle", (m_AssetPath.filename().string() + " import settings").c_str(), 50);
		ImGui::PopFont();


		ImGui::Columns(2);
		// Min filtering 
		ImGui::Text("Min filtering");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##minfilter", Tex2DParamToString(minFilt).c_str()))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = currMinString == minTypes[i];
				if (ImGui::Selectable(minTypes[i], &isSelected))
				{
					currMinString = minTypes[i];
					texture->SetMinFiltering(StringToTex2DParam(std::string(currMinString)));
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		// Mag filtering 
		ImGui::NextColumn();
		ImGui::Text("Mag filtering");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##magfilter", Tex2DParamToString(magFilt).c_str()))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = currMagString == magTypes[i];
				if (ImGui::Selectable(magTypes[i], &isSelected))
				{
					currMagString = magTypes[i];
					texture->SetMagFiltering(StringToTex2DParam(std::string(currMagString)));
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
			config.Filtering = StringToTex2DParam(currMinString);
			config.WrapMode = StringToTex2DParam(currWrapString);

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