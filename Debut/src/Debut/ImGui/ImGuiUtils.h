#pragma once
#include <imgui.h>
#include <imgui_internal.h>

#include <Debut/AssetManager/AssetManager.h>

#define IMGUI_ICON_DIR				u8"\ue000"
#define IMGUI_ICON_FILE				u8"\ue001"
#define IMGUI_ICON_MODEL			u8"\ue003"
#define IMGUI_ICON_MESH				u8"\ue004"
#define IMGUI_ICON_MATERIAL			u8"\ue005"
#define IMGUI_ICON_UNIMPORTED_MODEL	u8"\ue006"
#define IMGUI_ICON_ENTITY			u8"\ue007"

namespace Debut
{
	class Texture2D;
	class PhysicsMaterial2D;
	class AssetManager;

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
		

		static bool ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color = {0.1f, 0.2f, 0.4f, 1.0f});
		static void BoldText(const std::string& label);
		static bool Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret);
		static bool ImageTreeNode(const char* id, ImTextureID texture, bool open = false, bool selected = false);
		static std::string GetFileImguiIcon(const std::string& extension);

		static bool BeginDragDropSourceCustom(ImGuiDragDropFlags flags = 0);

		static UUID DragDestination(const std::string& label, const std::string& acceptedExtension, UUID currentID);

		template <typename T>
		static Ref<T> ImageDragDestination(uint32_t rendererID, ImVec2 size);
	};
}