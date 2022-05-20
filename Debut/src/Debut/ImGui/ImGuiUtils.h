#pragma once
#include <imgui.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Renderer/Texture.h>

namespace Debut
{
	class ImGuiUtils
	{
	public:
		static void StartColumns(uint32_t amount, std::vector<uint32_t> sizes);
		static void NextColumn();
		static void ResetColumns();
		static void VerticalSpace(uint32_t amount);

		static bool DragFloat(const std::string& label, float* value, float power, float min = -100000.0f, float max = 100000.0f);
		static void RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);
		static void RGBVec2(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);

		static bool ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color = {0.1, 0.2, 0.4, 1});
		static bool Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret);

		template <typename T>
		static UUID DragDestination(const std::string& label, const std::string& acceptedExtension, UUID currentID)
		{
			UUID ret = 0;

			std::string currentName = AssetManager::GetPath(currentID);
			currentName = currentName.substr(currentName.find_last_of("\\") + 1, currentName.size() - currentName.find_last_of("\\"));

			ImGui::PushID(label.c_str());
			ImGuiUtils::StartColumns(2, { 120, 280 });

			ImGui::Text(label.c_str());
			ImGui::SameLine();
			ImGui::NextColumn();

			// TODO: put name of selected material inside 
			ImGui::Button((currentName + "##" + label).c_str(), {ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f});
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path pathStr(path);

					if (pathStr.extension() == acceptedExtension)
					{
						Ref<T> selectedAsset = AssetManager::Request<T>(pathStr.string());
						ret = selectedAsset->GetID();
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGuiUtils::ResetColumns();
			ImGui::PopID();

			return ret;
		}
	};
}