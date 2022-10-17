#include <Debut/dbtpch.h>
#include <Debut/ImGui/ImGuiUtils.h>

#include <Debut/Rendering/Texture.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/AssetManager/AssetManager.h>

#include <yaml-cpp/yaml.h>

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

	void ImGuiUtils::Separator()
	{
		ImGui::Dummy({ 0, 3 });
		ImGui::Separator();
		ImGui::Dummy({ 0, 3 });
	}

	bool ImGuiUtils::DragFloat(const std::string& label, float* value, float power, float min, float max, uint32_t columnWidth)
	{
		bool ret = false;

		ImGui::PushID(label.c_str());

		ImGuiUtils::ResetColumns();
		ImGuiUtils::StartColumns(2, { (uint32_t)columnWidth, 200 });
		
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

	bool ImGuiUtils::Color3(const std::string& label, std::vector<float*> values)
	{
		bool ret = false;
		std::string pickerString = label + "-picker";
		std::string buttonString = label + "-button";
		const char* pickerLabel = pickerString.c_str();
		const char* buttonLabel = buttonString.c_str();

		ImGui::PushID(label.c_str());

		ImGuiUtils::ResetColumns();
		ImGuiUtils::StartColumns(2, { 150, 200 });
		ImColor color = { *values[0], *values[1], *values[2] };

		ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });

		ImGui::Text(label.c_str());
		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ImGui::SetNextItemWidth(200);
		if (ImGui::ColorButton(buttonLabel, color, 0, { ImGui::CalcItemWidth(), GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f }))
			ImGui::OpenPopup(pickerLabel);
		if (ImGui::BeginPopup(pickerLabel))
		{
			float vec[3] = { *values[0],*values[1],*values[2] };
			ret = ImGui::ColorPicker3("##picker", vec, ImGuiColorEditFlags_None);
			*values[0] = vec[0]; *values[1] = vec[1]; *values[2] = vec[2];
			ImGui::EndPopup();
		}

		ImGui::NextColumn();

		ImGui::PopStyleVar();
		ImGui::PopID();

		ImGuiUtils::ResetColumns();

		return ret;
	}

	bool ImGuiUtils::Color4(const std::string& label, std::vector<float*> values)
	{
		bool ret = false;

		ImGui::PushID(label.c_str());

		ImGuiUtils::ResetColumns();
		ImGuiUtils::StartColumns(2, { 150, 200 });

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

		ImGui::Text(label.c_str());
		ImGui::PopItemWidth();
		ImGui::NextColumn();

		float vec[4] = { *values[0],*values[1],*values[2],*values[3] };
		ret = ImGui::ColorEdit4(label.c_str(), vec, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		*values[0] = vec[0]; *values[1] = vec[1]; *values[2] = vec[2]; *values[3] = vec[3];
		ImGui::NextColumn();

		ImGui::PopItemWidth();
		ImGui::PopID();

		ImGuiUtils::ResetColumns();

		return ret;
	}

	void ImGuiUtils::RGBVec2(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue, uint32_t columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(id);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });
		ImGui::Text(id);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight, lineHeight };

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

	void ImGuiUtils::RGBVec3(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue, uint32_t columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(id);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		ImGui::Text(id);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight, lineHeight };

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

	void ImGuiUtils::RGBVec4(const char* id, std::vector<const char*>labels, std::vector<float*>values, float resetValue, uint32_t columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(id);

		uint32_t vecColumnSize = ((uint32_t)ImGui::GetContentRegionAvail().x - columnWidth) / 2.0f;

		ImGuiUtils::StartColumns(3, { columnWidth, vecColumnSize, vecColumnSize });

		ImGui::Text(id);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		ImGui::NextColumn();

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight, lineHeight };

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
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat("##X", values[0], 0.15f);
		
		ImGui::NextColumn();

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
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat("##Y", values[1], 0.15f);

		ImGui::NextColumn(); ImGui::NextColumn();

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
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat("##Z", values[2], 0.15f);
		
		ImGui::NextColumn();

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
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat("##W", values[3], 0.15f);
		ImGui::SameLine();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	bool ImGuiUtils::ImageButton(Ref<Texture2D> texture, ImVec2 size, ImVec4 color)
	{
		return ImGui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 1, 0 }, { 0, 1 }, -1, color);
	}

	template Ref<Texture2D> ImGuiUtils::ImageDragDestination(uint32_t, ImVec2, const char* id);
	template<typename T>
	Ref<T> ImGuiUtils::ImageDragDestination(uint32_t rendererID, ImVec2 size, const char* id)
	{
		Ref<T> selectedAsset = nullptr;

		if (id != 0)
			ImGui::PushID(id);
		// Accept PNG files to use as textures for the sprite renderer
		ImGui::ImageButton((ImTextureID)rendererID, size, { 0, 1 }, { 1, 0 });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path pathStr(path);

				if (pathStr.extension().compare(".png")==0 || pathStr.extension().compare(".jpg")==0 ||
					pathStr.extension().compare(".tga")==0 || pathStr.extension().compare(".jpeg")==0)
					selectedAsset = AssetManager::Request<T>(pathStr.string());
			}
			ImGui::EndDragDropTarget();
		}

		if (id != 0)
			ImGui::PopID();

		return selectedAsset;
	}

	UUID ImGuiUtils::DragDestination(const std::string& label, const std::string& acceptedExtension, UUID currentID)
	{
		UUID ret = 0;

		std::string currentName = AssetManager::GetPath(currentID);
		currentName = currentName.substr(currentName.find_last_of("\\") + 1, currentName.size() - currentName.find_last_of("\\"));

		ImGui::PushID(label.c_str());
		ImGuiUtils::StartColumns(2, { 120, 280 });

		ImGui::Text(label.c_str());
		ImGui::SameLine();
		ImGui::NextColumn();

		ImGui::Button((currentName + "##" + label).c_str(), { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f });
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
					else
					{
						meta.close();
						meta.open(AssetManager::s_MetadataDir + pathStr.stem().string() + ".meta");

						if (meta.good())
						{
							std::stringstream ss;
							ss << meta.rdbuf();
							YAML::Node metaData = YAML::Load(ss.str());
							ret = metaData["ID"].as<uint64_t>();
						}
					}
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGuiUtils::ResetColumns();
		ImGui::PopID();

		return ret;
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

	bool ImGuiUtils::BeginDragDropSourceCustom(ImGuiDragDropFlags flags)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiMouseButton mouse_button = ImGuiMouseButton_Left;

		bool source_drag_active = false;
		ImGuiID id = ImGui::GetHoveredID();
		if (id == 0)
			return false;
		ImGuiID source_id = id;
		ImGuiID source_parent_id = id;

		if (!(flags & ImGuiDragDropFlags_SourceExtern))
		{
			ImGui::SetActiveID(id, window);
			ImGui::KeepAliveID(id);

			source_drag_active = ImGui::IsMouseDragging(mouse_button);
			// Disable navigation and key inputs while dragging + cancel existing request if any
			ImGui::SetActiveIdUsingNavAndKeys();
		}
		else
		{
			window = NULL;
			source_id = ImHashStr("#SourceExtern");
			source_drag_active = true;
		}

		if (source_drag_active)
		{
			if (!g.DragDropActive)
			{
				IM_ASSERT(source_id != 0);
				ImGui::ClearDragDrop();
				ImGuiPayload& payload = g.DragDropPayload;
				payload.SourceId = source_id;
				payload.SourceParentId = source_parent_id;
				g.DragDropActive = true;
				g.DragDropSourceFlags = flags;
				g.DragDropMouseButton = mouse_button;
				if (payload.SourceId == g.ActiveId)
					g.ActiveIdNoClearOnFocusLoss = true;
			}
			g.DragDropSourceFrameCount = g.FrameCount;
			g.DragDropWithinSource = true;

			if (!(flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
			{
				// Target can request the Source to not display its tooltip (we use a dedicated flag to make this request explicit)
				// We unfortunately can't just modify the source flags and skip the call to BeginTooltip, as caller may be emitting contents.
				ImGui::BeginTooltip();
				if (g.DragDropAcceptIdPrev && (g.DragDropAcceptFlags & ImGuiDragDropFlags_AcceptNoPreviewTooltip))
				{
					ImGuiWindow* tooltip_window = g.CurrentWindow;
					tooltip_window->Hidden = tooltip_window->SkipItems = true;
					tooltip_window->HiddenFramesCanSkipItems = 1;
				}
			}

			if (!(flags & ImGuiDragDropFlags_SourceNoDisableHover) && !(flags & ImGuiDragDropFlags_SourceExtern))
				g.LastItemData.StatusFlags &= ~ImGuiItemStatusFlags_HoveredRect;

			return true;
		}
		return false;
	}

	bool ImGuiUtils::ImageTreeNode(const char* label, ImTextureID texture, bool open, bool selected)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		ImGuiID id = window->GetID(label);
		ImVec2 pos = window->DC.CursorPos;
		ImRect bb(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + g.FontSize + g.Style.FramePadding.y * 2));
		bool opened = open;
		bool hovered, held;
		bool ret = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClickRelease);
		uint32_t colorIndex;
		if (held || hovered || selected)
			colorIndex = ImGuiCol_HeaderHovered;
		else
			colorIndex = ImGuiCol_WindowBg;

		
		if (ret)
			window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
		if (hovered || held || opened || selected)
			window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(
				ImGui::GetStyle().Colors[colorIndex]));

		// Icon, text
		float button_sz = g.FontSize + g.Style.FramePadding.y * 2;
		window->DrawList->AddImage(texture, {pos.x, pos.y}, {pos.x + button_sz, pos.y + button_sz}, { 0, 1 }, { 1, 0 });
		ImGui::RenderText(ImVec2(pos.x + button_sz + g.Style.ItemInnerSpacing.x, pos.y + g.Style.FramePadding.y), label);

		ImGui::ItemSize(bb, g.Style.FramePadding.y);
		ImGui::ItemAdd(bb, id);

		if (opened || ret)
			ImGui::TreePush(label);

		return ret;
	}

	std::string ImGuiUtils::GetFileImguiIcon(const std::string& icon)
	{
		std::string ret = u8"\ue000";

		if (icon == ".mat")
			ret = IMGUI_ICON_MATERIAL;
		else if (icon == ".mesh")
			ret = IMGUI_ICON_MESH;
		else if (icon == ".model")
			ret = IMGUI_ICON_MODEL;
		else if (icon == ".obj" || icon == ".fbx")
			ret = IMGUI_ICON_UNIMPORTED_MODEL;
		else if (icon == ".dir")
			ret = IMGUI_ICON_DIR;
		else
			ret = IMGUI_ICON_FILE;

		return ret;
	}
}