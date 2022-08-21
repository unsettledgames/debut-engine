#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include <filesystem>

#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Rendering/Texture.h>

#define IMGUI_ICON_DIR				u8"\ue000"
#define IMGUI_ICON_FILE				u8"\ue001"
#define IMGUI_ICON_MODEL			u8"\ue003"
#define IMGUI_ICON_MESH				u8"\ue004"
#define IMGUI_ICON_MATERIAL			u8"\ue005"
#define IMGUI_ICON_UNIMPORTED_MODEL	u8"\ue006"
#define IMGUI_ICON_ENTITY			u8"\ue007"

namespace Debut
{
	class ImGuiUtils
	{
	public:
		static void StartColumns(uint32_t amount, std::vector<uint32_t> sizes);
		static void NextColumn();
		static void ResetColumns();
		static void VerticalSpace(uint32_t amount);
		static void Separator();

		static bool DragFloat(const std::string& label, float* value, float power, float min = -100000.0f, float max = 100000.0f);
		static void RGBVec2(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);
		static void RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);
		static void RGBVec4(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);
		

		static bool ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color = {0.1, 0.2, 0.4, 1});
		static void BoldText(const std::string& label);
		static bool Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret);
		static bool ImageTreeNode(const char* id, ImTextureID texture, bool open = false, bool selected = false);
		static std::string GetFileImguiIcon(const std::string& extension);

		static bool BeginDragDropSourceCustom(ImGuiDragDropFlags flags = 0);

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

			ImGui::Button((currentName + "##" + label).c_str(), {ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f});
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path pathStr(path);

					// Instead of asking for the asset, load the .meta file and extract the ID
					if (pathStr.extension() == acceptedExtension)
					{
						std::ifstream meta(pathStr.string() + ".meta");
						if (meta.good())
						{
							std::stringstream ss;
							ss << meta.rdbuf();
							YAML::Node metaData = YAML::Load(ss.str());

							ret = metaData["ID"].as<uint64_t>();
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGuiUtils::ResetColumns();
			ImGui::PopID();

			return ret;
		}

		template <typename T>
		static Ref<T> ImageDragDestination(uint32_t rendererID, ImVec2 size)
		{
			Ref<T> selectedAsset = nullptr;
			// Accept PNG files to use as textures for the sprite renderer
			ImGui::ImageButton((ImTextureID)rendererID, size, { 0, 1 }, { 1, 0 });
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path pathStr(path);

					if (pathStr.extension() == ".png")
						selectedAsset = AssetManager::Request<T>(pathStr.string());
				}
				ImGui::EndDragDropTarget();
			}

			return selectedAsset;
		}
	};
}