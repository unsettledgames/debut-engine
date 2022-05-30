#include <Debut/dbtpch.h>
#include <Debut/ImGui/ImGuiUtils.h>

#include <Debut/AssetManager/AssetManager.h>
#include <filesystem>
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

	void ImGuiUtils::VerticalSpace(uint32_t amount)
	{
		ImGui::Dummy({0, (float)amount});
	}

	bool ImGuiUtils::DragFloat(const std::string& label, float* value, float power, float min, float max)
	{
		bool ret = false;

		ImGui::PushID(label.c_str());

		ImGuiUtils::ResetColumns();
		ImGuiUtils::StartColumns(2, { 150, 200 });
		
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		ImGui::Text(label.c_str());
		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ret = ImGui::DragFloat(("##"+label).c_str(), value, power, min, max);
		ImGui::NextColumn();

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PopID();

		ImGuiUtils::ResetColumns();

		return ret;
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

	void ImGuiUtils::RGBVec2(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue, uint32_t columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(id);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(id);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
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
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void ImGuiUtils::RGBVec4(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue, uint32_t columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(id);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(id);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
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
		ImGui::SameLine();

		// Grey W component
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.2f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("W", buttonSize))
			*values[3] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##W", values[3], 0.15f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	bool ImGuiUtils::ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color)
	{
		return ImGui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 1, 0 }, { 0, 1 }, -1, color);
	}

	void ImGuiUtils::BoldText(const std::string& label)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushFont(boldFont);
		ImGui::Text(label.c_str());
		ImGui::PopFont();
	}

	bool ImGuiUtils::Combo(const char* id, const char* selectables[], uint32_t nSelectables, const char** currSelected, const char** ret)
	{
		bool changed = false;
		ImGuiUtils::ResetColumns();
		ImGuiUtils::StartColumns(2, { 100, 400});
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushID(id);
		ImGui::Text(id);
		ImGui::PopItemWidth();

		ImGui::NextColumn();

		if (ImGui::BeginCombo(("##"+std::string(id)).c_str(), *currSelected))
		{
			for (int i = 0; i < nSelectables; i++)
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
		
		ImGui::PopItemWidth();
		ImGui::PopID();
		ImGuiUtils::ResetColumns();

		return changed;
	}
}