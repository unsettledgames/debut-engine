#include "PropertiesPanel.h"
#include "Utils/EditorCache.h"
#include <Debut/Renderer/Texture.h>
#include <imgui.h>

namespace Debutant
{
	static std::string Tex2DParamToString(Texture2DParameter parameter)
	{
		switch (parameter)
		{
		case Texture2DParameter::FILTERING_LINEAR: return "Linear";
		case Texture2DParameter::FILTERING_POINT: return "Point";
		case Texture2DParameter::WRAP_CLAMP: return "Clamp";
		case Texture2DParameter::WRAP_REPEAT: return "Repeat";
		}

		Log.AppWarn("Texture parameter {0} not supported", (uint32_t)parameter);
		return "";
	}
	static Texture2DParameter StringToTex2DParam(const std::string& param)
	{
		if (param == "Linear") return Texture2DParameter::FILTERING_LINEAR;
		if (param == "Point") return Texture2DParameter::FILTERING_POINT;
		if (param == "Clamp") return Texture2DParameter::WRAP_CLAMP;
		if (param == "Repeat") return Texture2DParameter::WRAP_REPEAT;

		Log.AppWarn("Texture parameter {0} not supported", param);
		return Texture2DParameter::NONE;
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
		
		Texture2DParameter magFilt = texture->GetMagFiltering();
		Texture2DParameter minFilt = texture->GetMinFiltering();
		Texture2DParameter wrapMode = texture->GetWrapMode();

		const char* currMagString = Tex2DParamToString(magFilt).c_str();
		const char* currMinString = Tex2DParamToString(magFilt).c_str();
		const char* currWrapString = Tex2DParamToString(wrapMode).c_str();

		const char* magTypes[] = { "Linear", "Point" };
		const char* minTypes[] = { "Linear", "Point" };
		const char* wrapTypes[] = { "Clamp", "Repeat" };
	

		// Bind the imported texture to the gameplay texture.

		// Min filtering 
		ImGui::Text("Min filtering");
		ImGui::SameLine();
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
		ImGui::Text("Mag filtering");
		ImGui::SameLine();
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
		ImGui::Text("Wrap mode");
		ImGui::SameLine();
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

		ImGui::Text("Texture preview");
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImVec2 textureSize;

		textureSize = { windowSize.x, windowSize.x / texture->GetAspectRatio() };
		ImGui::Image((ImTextureID)texture->GetRendererID(), textureSize, { 0, 0 }, { 1, -1 });
	}
}