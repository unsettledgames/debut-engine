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

#define IMGUI_ICON_GIZMO_GLOBAL		u8"\ue008"
#define IMGUI_ICON_GIZMO_LOCAL		u8"\ue009"

#define IMGUI_ICON_TRANSLATE		u8"\ue00a"
#define IMGUI_ICON_ROTATE			u8"\ue00b"
#define IMGUI_ICON_SCALE			u8"\ue00c"

#define IMGUI_ICON_PLAY				u8"\ue00d"
#define IMGUI_ICON_STOP				u8"\ue00e"


namespace Debut
{
	class Texture2D;
	class PhysicsMaterial2D;
	class AssetManager;

	class ScopedStyleVar
	{
	public:
		ScopedStyleVar(ImGuiStyleVar var, float val)			{ ImGui::PushStyleVar(var, val); }
		ScopedStyleVar(ImGuiStyleVar var, const ImVec2& val)	{ ImGui::PushStyleVar(var, val); }

		~ScopedStyleVar() { ImGui::PopStyleVar(); }
	};

	class ScopedStyleColor
	{
	public:
		ScopedStyleColor(ImGuiCol col, const ImVec4& val) { ImGui::PushStyleColor(col, val); }

		~ScopedStyleColor() { ImGui::PopStyleColor(); }
	};

	class ImGuiUtils
	{
	public:
		static void StartColumns(uint32_t amount, std::vector<uint32_t> sizes);
		static void NextColumn();
		static void ResetColumns();
		static void VerticalSpace(uint32_t amount);
		static void Separator();

		static bool DragFloat(const std::string& label, float* value, float power, float min = -100000.0f, float max = 100000.0f, uint32_t columnWidth = 130);
		static bool DragInt(const std::string& label, int* value, float power, float min = -100000.0f, float max = 100000.0f, uint32_t columnWidth = 130);
		static bool Color3(const std::string& label, std::vector<float*> values);
		static bool Color4(const std::string& label, std::vector<float*> values);

		static void RGBVec2(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 130);
		static void RGBVec4(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 130);
		static void RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 130);

		static bool ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color = {0.1f, 0.2f, 0.4f, 1.0f});
		static void BoldText(const std::string& label);
		static bool Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret);
		static bool ImageTreeNode(const char* id, ImTextureID texture, bool open = false, bool selected = false);
		static std::string GetFileImguiIcon(const std::string& extension);

		static bool BeginDragDropSourceCustom(ImGuiDragDropFlags flags = 0);

		static UUID DragDestination(const std::string& label, const std::string& acceptedExtension, UUID currentID);

		template <typename T>
		static Ref<T> ImageDragDestination(uint32_t rendererID, ImVec2 size, const char* id = 0);
	};
}