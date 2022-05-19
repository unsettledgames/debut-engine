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

		static bool DragFloat(const std::string& label, float* value, float power, float min = -100000.0f, float max = 100000.0f);
		static void RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);
		static void RGBVec2(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue = 0, uint32_t columnWidth = 100);

		static bool ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color = {0.1, 0.2, 0.4, 1});
		static bool Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret);

		template <typename T>
		static Ref<T> DragDestination(const std::string& label, const std::string& acceptedExtension, const std::string& current);
	};
}