#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Renderer/Texture.h>
#include <filesystem>
#include <Debut/ImGui/ImGuiUtils.h>

namespace Debut
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		EditorCache::Textures().Put("assets\\textures\\empty_texture.png", Texture2D::Create(1, 1));
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::SetSelectedEntity(const Entity& entity)
	{
		if ((int)(entt::entity)entity != -1)
			m_SelectionContext = entity;
		else
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

		// Right click menu
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("New Entity"))
				m_SelectionContext = m_Context->CreateEntity();
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Inspector");

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

		// TODO: HACKY
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
			if (ImGui::MenuItem("Duplicate"))
				m_Context->DuplicateEntity(entity);

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
		if (!entity.HasComponent<T>())
			return;
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap 
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

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
			DrawAddComponentEntry<CameraComponent>("Camera");
			DrawAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DrawAddComponentEntry<Rigidbody2DComponent>("Rigidbody2D");
			DrawAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DrawAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");

			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				glm::vec3 rotDeg = glm::degrees(component.Rotation);
				ImGuiUtils::RGBVec3("Position", { "X", "Y", "Z" }, {&component.Translation.x, &component.Translation.y, &component.Translation.z});
				ImGuiUtils::RGBVec3("Rotation", { "X", "Y", "Z" }, {&rotDeg.x, &rotDeg.y, &rotDeg.z});
				ImGuiUtils::RGBVec3("Scale", { "X", "Y", "Z" }, {&component.Scale.x,&component.Scale.y, &component.Scale.z}, 1);

				component.Rotation = glm::radians(rotDeg);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				const char* projectionTypes[] = { "Perspective", "Orthographic" };
				const char* currProjType = projectionTypes[(int)camera.GetProjectionType()];
				const char* finalProjType = nullptr;

				if (ImGuiUtils::Combo("Projection", projectionTypes, 2, &currProjType, &finalProjType))
					camera.SetProjectionType(SceneCamera::StringToProjType(finalProjType));
				ImGui::Checkbox("Set as primary", &component.Primary);
				ImGui::Dummy({0.0f, 0.5f});

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float perspFOV = glm::degrees(camera.GetPerspFOV());
					if (ImGuiUtils::DragFloat("Vertical FOV", &perspFOV, 0.15f))
						camera.SetPerspFOV(glm::radians(perspFOV));

					float perspNear = camera.GetPerspNearClip();
					if (ImGuiUtils::DragFloat("Near clip", &perspNear, 0.15f))
						camera.SetPerspNearClip(perspNear);

					float perspFar = camera.GetPerspFarClip();
					if (ImGuiUtils::DragFloat("Far clip", &perspFar, 0.15f))
						camera.SetPerspFarClip(perspFar);
				}
				else
				{
					float orthoSize = camera.GetOrthoSize();
					if (ImGuiUtils::DragFloat("Size", &orthoSize, 0.15f))
						camera.SetOrthoSize(orthoSize);

					float orthoNear = camera.GetOrthoNearClip();
					if (ImGuiUtils::DragFloat("Near clip", &orthoNear, 0.15f))
						camera.SetOrthoNearClip(orthoNear);

					float orthoFar = camera.GetOrthoFarClip();
					if (ImGuiUtils::DragFloat("Far clip", &orthoFar, 0.15f))
						camera.SetOrthoFarClip(orthoFar);
				}
			});


		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				// Color
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), 0.15f);
				// Texture
				ImTextureID buttonTexture;
				// Use a blank texture if the user hasn't already set one, otherwise use the one submitted by the user	
				buttonTexture = component.Texture == nullptr ?
					(ImTextureID)EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID() :
					(ImTextureID)component.Texture->GetRendererID();

				ImGuiUtils::StartColumns(3, {80, 100, 100});

				// Accept PNG files to use as textures for the sprite renderer
				ImGui::ImageButton(buttonTexture, ImVec2(64.0f, 64.0f), { 0, 1 }, { 1, 0 });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path pathStr(path);

						if (pathStr.extension() == ".png")
						{
							Ref<Texture2D> selectedTexture = AssetManager::Request<Texture2D>(pathStr.string());
							component.Texture = selectedTexture;
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::NextColumn();

				ImGui::SameLine();
				ImGuiUtils::DragFloat("Tiling factor", &component.TilingFactor, 0.1f);

			});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currBodyType = bodyTypeStrings[(int)component.Type];
				const char* finalBodyType = nullptr;

				if (ImGuiUtils::Combo("Body type", bodyTypeStrings, 3, &currBodyType, &finalBodyType))
					component.Type = Rigidbody2DComponent::StrToRigidbody2DType(finalBodyType);

				ImGui::Checkbox("Fixed rotation", &component.FixedRotation);
			});
		
		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::Dummy({ 0.0f, 5.0f });

				ImGuiUtils::RGBVec2("Offset", { "X", "Y" }, { &component.Offset.x, &component.Offset.y });
				ImGuiUtils::RGBVec2("Size", { "X", "Y" }, { &component.Size.x, &component.Size.y });

				ImGuiUtils::VerticalSpace(10);

				UUID material = ImGuiUtils::DragDestination<PhysicsMaterial2D>("Physics material", ".physmat2d", component.Material);
				if (material)
					component.Material = material;
			});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
			{
				ImGuiUtils::RGBVec2("Offset", { "X", "Y" }, { &component.Offset.x, &component.Offset.y });
				ImGuiUtils::DragFloat("Radius", &component.Radius, 0.1f);

				ImGui::Dummy({ 0.0f, 10.0f });
				ImGui::Separator();
				ImGui::Dummy({ 0.0f, 10.0f });

				ImGuiUtils::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGuiUtils::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGuiUtils::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGuiUtils::DragFloat("Restitution threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});
	}
}