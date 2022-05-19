#include <Debut/dbtpch.h>
#include <Debut/ImGui/ImGuiUtils.h>
#include <imgui_internal.h>

namespace Debut
{
	void ImGuiUtils::StartColumns(uint32_t amount, std::vector<uint32_t> sizes)
	{
		ImGui::Columns(amount);
		for (uint32_t i = 0; i < amount; i++)
			ImGui::SetColumnWidth(i, sizes[i]);
	}

	void ImGuiUtils::NextColumn()
	{
		ImGui::NextColumn();
	}

	void ImGuiUtils::ResetColumns()
	{
		ImGui::Columns(1);
	}

	void ImGuiUtils::DragFloats(int amount, const std::string labels[], float* values[], float mins[], float maxs[], float powers[])
	{
		for (uint32_t i = 0; i < amount; i++)
		{
			ImGui::PushID(labels[i].c_str());
			ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

			ImGui::Text(labels[i].c_str());
			ImGui::NextColumn();

			ImGui::DragFloat("##", values[i], mins[i], maxs[i], powers[i]);
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();
			ImGui::PopID();
		}
	}

	void ImGuiUtils::RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue, uint32_t columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(id);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(id);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		// Red X component
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			*values[0] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", values[0], 0.15f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// Green Y component
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			*values[1] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", values[1], 0.15f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// Blue Z component
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.4f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			*values[2] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", values[2], 0.15f);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	bool ImGuiUtils::ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color)
	{
		return ImGui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 1, 0 }, { 0, 1 }, -1, color);
	}

	bool ImGuiUtils::Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret)
	{
		bool changed = false;

		if (ImGui::BeginCombo(id, *currSelected))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = *currSelected == selectables[i];
				if (ImGui::Selectable(selectables[i], &isSelected))
				{
					*ret = selectables[i];

					if (*ret != *currSelected)
					{
						changed = true;
						*currSelected = selectables[i];
					}
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		return changed;
	}
}