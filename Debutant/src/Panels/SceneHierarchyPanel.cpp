#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Texture.h>
#include <filesystem>
#include <Debut/ImGui/ImGuiUtils.h>
#include <entt.hpp>

/*
	TODO
		- Change parents by dragging objects around
			- Save the last hovered item
			- If the user drops when the item is not hovered
				- Detect if they dropped above or below the last hovered item
				- if above, put above, if below, put below (on the same tree level)
		- Save objects in the right order
		- Move objects in the hierarchy
*/

namespace Debut
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		EditorCache::Textures().Put("assets\\textures\\empty_texture.png", Texture2D::Create(1, 1));
		m_CachedSceneGraph = new EntitySceneNode();
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
		delete m_CachedSceneGraph;
		for (auto e : m_ExistingEntities)
			delete e.second;
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

		for (uint32_t i = 0; i < m_CachedSceneGraph->Children.size(); i++)
			DrawEntityNode(*m_CachedSceneGraph->Children[i]);

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right click menu
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("New Entity"))
			{
				m_SelectionContext = m_Context->CreateEntity({});
				m_ExistingEntities[m_SelectionContext] = new EntitySceneNode(false, m_SelectionContext);
				RebuildSceneGraph();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (m_DraggingEntity && !m_DroppedOnEntity && m_LastHoveredEntity)
			{
				// Take care of repositioning entities
				

				m_LastHoveredEntity = {};
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(EntitySceneNode& node)
	{
		if (m_RebuiltGraph)
			return;

		bool entityDeleted = false;
		auto& tc = node.EntityData.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = (m_SelectionContext == node.EntityData ? ImGuiTreeNodeFlags_OpenOnArrow : 0);
		flags |= ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;
		
		// Don't let the user expand a node if it doesn't have children
		if (node.Children.size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;
		// Highlight the selected node
		if (node.EntityData == m_SelectionContext)
			flags |= ImGuiTreeNodeFlags_Selected;

		// Color the node differently if it's selected
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
		if ((uint32_t)node.EntityData != (uint32_t)m_SelectionContext)
			ImGui::PushStyleColor(ImGuiCol_Header, { 0.0, 0.0, 0.0, 0.0 });
		else
			ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
		std::string entityName = node.Children.size() == 0 ? (IMGUI_ICON_ENTITY + std::string("  ") + tc.Name) : tc.Name;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)node.EntityData, flags, entityName.c_str());
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		if (ImGui::IsItemClicked())
			m_SelectionContext = node.EntityData;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
			m_LastHoveredEntity = node.EntityData;

		// Drag n drop to parent entities
		if (ImGui::BeginDragDropSource())
		{
			uint32_t id = node.EntityData;
			ImGui::SetDragDropPayload("SCENE_HIERARCHY_ENTITY", (void*)&id, sizeof(id), ImGuiCond_Once);
			ImGui::EndDragDropSource();

			m_DroppedOnEntity = false;
			m_DraggingEntity = true;
		}
		else if (ImGui::BeginDragDropTarget())
		{
			// Set the current node as the payload's parent
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY");
			if (payload != nullptr)
			{
				uint32_t entityId = *((uint32_t*)payload->Data);
				Entity child = { (entt::entity)entityId, m_Context.get()};

				// Check if child is parent of this entity
				Entity parent = node.EntityData;
				bool canParent = true;
				while (parent && canParent)
				{
					if ((uint32_t)parent == (uint32_t)child)
						canParent = false;
					parent = parent.Transform().Parent;
				}
				
				if (canParent)
				{
					child.Transform().SetParent(node.EntityData);
					RebuildSceneGraph();
				}

				m_DroppedOnEntity = true;
				m_DraggingEntity = false;
				m_LastHoveredEntity = {};
			}
			
			ImGui::EndDragDropTarget();
		}

		// One approach to moving entities: it has problems
		/*ImGui::InvisibleButton("Invisible", {ImGui::GetContentRegionAvail().x, 1});
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
			m_HoveringInvisibleEntityButton = true;
		else
			m_HoveringInvisibleEntityButton = false;
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY");
			ImGui::EndDragDropTarget();
		}
		*/
		// Right click on blank space
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("New Entity"))
			{
				m_SelectionContext = m_Context->CreateEntity(node.EntityData);
				m_ExistingEntities[m_SelectionContext] = new EntitySceneNode(false, m_SelectionContext);
				RebuildSceneGraph();

				ImGui::EndPopup();
				if (opened)
					ImGui::TreePop();	
				return;
			}
			if (ImGui::MenuItem("Duplicate"))
			{
				m_Context->DuplicateEntity(node.EntityData);
				RebuildSceneGraph();

				ImGui::EndPopup();
				if (opened)
					ImGui::TreePop();
				return;
			}
			if (ImGui::MenuItem("Destroy"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (entityDeleted)
		{
			DestroySceneNode(node);
			RebuildSceneGraph();

			if (opened)
				ImGui::TreePop();
			return;
		}

		if (opened)
		{
			for (uint32_t i = 0; i < node.Children.size(); i++)
				DrawEntityNode(*node.Children[i]);
			ImGui::TreePop();
		}

		m_RebuiltGraph = false;
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
			DrawAddComponentEntry<MeshRendererComponent>("Mesh Renderer");

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

		
		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component)
			{
				ImGuiUtils::StartColumns(2, { 100, (uint32_t)ImGui::GetContentRegionAvail().x - 100 });
				MeshMetadata meshData = Mesh::GetMetadata(component.Mesh);
				MaterialMetadata materialData = Material::GetMetadata(component.Material);

				// Mesh reference
				ImGui::LabelText("##meshlabel", "Mesh");
				ImGui::NextColumn();

				ImGui::Button((meshData.Name + "##mesh").c_str(), { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path pathStr(path);

						if (pathStr.extension() == ".mesh")
						{
							pathStr = pathStr.replace_extension();
							std::ifstream meta(AssetManager::s_MetadataDir + pathStr.string() + ".meta");

							if (meta.good())
							{
								std::stringstream ss;
								ss << meta.rdbuf();
								YAML::Node metaData = YAML::Load(ss.str());

								component.Mesh = metaData["ID"].as<uint64_t>();
							}
						}
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::NextColumn();

				// Material reference
				// Mesh reference
				ImGui::LabelText("##materiallabel", "Material");
				ImGui::NextColumn();

				ImGui::Button((materialData.Name + "##material").c_str(), { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path pathStr(path);

						if (pathStr.extension() == ".mat")
						{
							pathStr = pathStr.replace_extension();
							std::ifstream meta(AssetManager::s_MetadataDir + pathStr.string() + ".meta");

							if (meta.good())
							{
								std::stringstream ss;
								ss << meta.rdbuf();
								YAML::Node metaData = YAML::Load(ss.str());

								component.Material = metaData["ID"].as<uint64_t>();
							}
						}
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::NextColumn();

				ImGuiUtils::ResetColumns();
			});


		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				// Color
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), 0.15f);
				// Texture
				ImTextureID buttonTexture;
				// Use a blank texture if the user hasn't already set one, otherwise use the one submitted by the user	
				buttonTexture = component.Texture == 0 ?
					(ImTextureID)EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID() :
					(ImTextureID)AssetManager::Request<Texture2D>(component.Texture)->GetRendererID();

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
							component.Texture = selectedTexture->GetID();
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

				UUID material = ImGuiUtils::DragDestination("Physics material", ".physmat2d", component.Material);
				if (material != 0)
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

	void SceneHierarchyPanel::DestroySceneNode(EntitySceneNode& node)
	{
		// Destroy children
		for (uint32_t i = 0; i < node.Children.size(); i++)
			DestroySceneNode(*node.Children[i]);

		m_Context->DestroyEntity(node.EntityData);
		if (m_SelectionContext == node.EntityData)
			m_SelectionContext = {};
	}


	void SceneHierarchyPanel::RebuildSceneGraph()
	{
		delete m_CachedSceneGraph;
		m_CachedSceneGraph = new EntitySceneNode();

		EntitySceneNode scene(true, {});
		auto transforms = m_Context->m_Registry.view<TransformComponent>();

		for (auto entity : transforms)
			m_ExistingEntities[entity]->Children.resize(0);

		for (auto entity : transforms)
		{
			auto& transform = transforms.get<TransformComponent>(entity);

			// If the object doesn't have a parent, then the parent is the root node
			if (transform.Parent)
			{
				entt::entity parentEntity = entt::to_entity(m_Context->m_Registry, transform.Parent);
				EntitySceneNode* currEntity = m_ExistingEntities[entity];
				std::vector<EntitySceneNode*> children = m_ExistingEntities[transform.Parent]->Children;

				currEntity->IndexInNode = children.size() > 0 ? children[children.size() - 1]->IndexInNode + 1 : 0;
				m_ExistingEntities[transform.Parent]->Children.push_back(m_ExistingEntities[entity]);
			}
		}

		uint32_t i = 0;
		for (auto entity : m_ExistingEntities)
		{
			if (!entity.second->EntityData.Transform().Parent)
			{
				m_CachedSceneGraph->Children.push_back(entity.second);
				entity.second->IndexInNode = i;
				i++;
			}
		}
	}
}