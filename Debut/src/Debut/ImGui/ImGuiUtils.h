#pragma once
#include <imgui.h>
#include <Debut/Renderer/Texture.h>

namespace Debut
{
	class ImGuiUtils
	{
	public:
		static void StartColumns(uint32_t amount, std::vector<uint32_t> sizes);
		static void NextColumn();
		static void ResetColumns();

		static void DragFloats(int amount, const std::string labels[], float* values[], float mins[], float maxs[], float powers[]);
		static void RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);
		static bool ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color = {0.1, 0.2, 0.4, 1});
		static bool Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret);
	};
}