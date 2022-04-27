#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>

namespace Debut
{
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
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
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.15f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// Green Y component
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.15f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// Blue Z component
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.4f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.15f);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([=](auto entity)
		{
			Entity entt = { entity, m_Context.get() };
			DrawEntityNode(entt);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("New Entity"))
				m_SelectionContext = m_Context->CreateEntity();

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity& entity)
	{
		bool entityDeleted = false;
		auto& tc = entity.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_OpenOnArrow : 0);
		flags |= ImGuiTreeNodeFlags_Selected;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		// HACKY
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tc.Name.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		// Right click on blank space
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Destroy"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGui::Text("TODO: transform trees");
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	template<typename T, typename UIFunction>
	void DrawComponent(const std::string& name, Entity& entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap 
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			bool remove = false;			
			ImVec2 availRegion = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			ImGui::SameLine(availRegion.x - lineHeight * 0.5f);
			if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
				ImGui::OpenPopup("ComponentSettings");

			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					remove = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				auto& component = entity.GetComponent<T>();
				uiFunction(component);
				ImGui::TreePop();
			}

			if (remove)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity& entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>();
			
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, tag.Name.c_str(), tag.Name.size());

			if (ImGui::InputText("##", buffer, sizeof(buffer)))
			{
				tag.Name = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (ImGui::MenuItem("Camera"))
			{
				m_SelectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer"))
			{
				m_SelectionContext.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				glm::vec3 rotDeg = glm::degrees(component.Rotation);

				DrawVec3Control("Position", component.Translation);
				DrawVec3Control("Rotation", rotDeg);
				DrawVec3Control("Scale", component.Scale, 1);

				component.Rotation = glm::radians(rotDeg);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				const char* projectionTypes[] = { "Perspective", "Orthographic" };
				const char* currProjType = projectionTypes[(int)camera.GetProjectionType()];

				if (ImGui::BeginCombo("Projection", projectionTypes[(int)camera.GetProjectionType()]))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currProjType == projectionTypes[i];
						if (ImGui::Selectable(projectionTypes[i], &isSelected))
						{
							currProjType = projectionTypes[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Checkbox("Set as primary", &component.Primary);

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float perspFOV = glm::degrees(camera.GetPerspFOV());
					if (ImGui::DragFloat("Vertical FOV", &perspFOV, 0.15f))
						camera.SetPerspFOV(glm::radians(perspFOV));

					float perspNear = camera.GetPerspNearClip();
					if (ImGui::DragFloat("Near clip", &perspNear, 0.15f))
						camera.SetPerspNearClip(perspNear);

					float perspFar = camera.GetPerspFarClip();
					if (ImGui::DragFloat("Far clip", &perspFar, 0.15f))
						camera.SetPerspFarClip(perspFar);
				}
				else
				{
					float orthoSize = camera.GetOrthoSize();
					if (ImGui::DragFloat("Size", &orthoSize, 0.15f))
						camera.SetOrthoSize(orthoSize);

					float orthoNear = camera.GetOrthoNearClip();
					if (ImGui::DragFloat("Near clip", &orthoNear, 0.15f))
						camera.SetOrthoNearClip(orthoNear);

					float orthoFar = camera.GetOrthoFarClip();
					if (ImGui::DragFloat("Far clip", &orthoFar, 0.15f))
						camera.SetOrthoFarClip(orthoFar);
				}
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), 0.15f);
			});
	}

}