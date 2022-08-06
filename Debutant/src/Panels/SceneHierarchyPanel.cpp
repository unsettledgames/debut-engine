#include "SceneHierarchyPanel.h"
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
			- Edge cases:
				- Crashing when moving from a parent to the top level in the same subtree
				- Can't unparent to the top level
		- Fix entity destruction
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

		m_HoveringEntity = false;
		m_LastMousePos = ImGui::GetMousePos();

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
				RebuildSceneGraph();
			}
			ImGui::EndPopup();
		}

		// Take care of repositioning entities
		if (m_DraggingEntity && !m_DroppedOnEntity && m_LastHoveredEntity)
		{
			ImVec2 highlightOffset = {};
			ImVec2 currMousePos = m_LastMousePos;

			if (currMousePos.y < m_LastHoveredMousePos.y)
				highlightOffset = { 0, -m_LastItemSize.y / 2.0f };
			else
				highlightOffset = { 0, m_LastItemSize.y / 2.0f };

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				// Get the dragged entity
				const ImGuiPayload* payload = ImGui::GetDragDropPayload();
				if (payload != nullptr)
				{
					uint32_t entityId = *((uint32_t*)payload->Data);
					Log.CoreInfo("Moved id: {0}", Entity((entt::entity)entityId, m_Context.get()).GetComponent<TagComponent>().Name);
					uint32_t parentId = m_LastHoveredEntity.Transform().Parent;
					if (parentId != -1)
						Log.CoreInfo("Moved id: {0}", Entity((entt::entity)parentId, m_Context.get()).GetComponent<TagComponent>().Name);

					// Place the dragged entity above or below the last hovered item
					if (currMousePos.y < m_LastHoveredMousePos.y)
						ChangeEntityOrder(entityId, m_ExistingEntities[(entt::entity)m_LastHoveredEntity]->IndexInNode, parentId);
					else
						ChangeEntityOrder(entityId, m_ExistingEntities[(entt::entity)m_LastHoveredEntity]->IndexInNode + 1, parentId);

					m_LastHoveredEntity = {};
					m_DraggingEntity = false;
				}
			}

			if (m_LastHoveredEntity && !m_HoveringEntity)
			{
				// Highlight the place where objects will be put
				ImGui::GetWindowDrawList()->AddLine(
					ImVec2(m_LastHoveredItemPos.x - m_LastItemSize.x / 2.0f, m_LastHoveredItemPos.y + highlightOffset.y),
					ImVec2(m_LastHoveredItemPos.x + m_LastItemSize.x / 2.0f, m_LastHoveredItemPos.y + highlightOffset.y),
					ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_DragDropTarget]), 2.0f
				);
			}
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);		

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

		// Get position and size data, then render the tree node
		ImVec2 hoveredPos = ImGui::GetCursorPos();
		hoveredPos = { hoveredPos.x + 2 + ImGui::GetWindowPos().x, hoveredPos.y + 2 + ImGui::GetWindowPos().y };
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)node.EntityData, flags, entityName.c_str());
		ImVec2 rectMin = ImGui::GetItemRectMin();
		ImVec2 rectMax = ImGui::GetItemRectMax();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		// Setup for selection and entity dragging
		if (ImGui::IsItemClicked())
			m_SelectionContext = node.EntityData;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && 
			m_DraggingEntity && (uint32_t)m_SelectionContext != (uint32_t)node.EntityData)
		{
			m_HoveringEntity = true;
			m_LastHoveredEntity = node.EntityData;
			m_LastHoveredMousePos = m_LastMousePos;
			m_LastItemSize = ImGui::GetItemRectSize();
			m_LastHoveredItemPos = {
				(rectMin.x + rectMax.x) / 2.0f,
				(rectMin.y + rectMax.y) / 2.0f
			};
		}

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

		// Right click on blank space
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("New Entity"))
			{
				m_SelectionContext = m_Context->CreateEntity(node.EntityData);
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

		// Add entity entries
		auto transforms = m_Context->m_Registry.view<TransformComponent>();
		// Add entity entries
		for (auto entity : transforms)
		{
			if (m_ExistingEntities.find(entity) == m_ExistingEntities.end())
				m_ExistingEntities[entity] = new EntitySceneNode(false, { entity, m_Context.get() });
		}

		// Reset children, gather data
		for (auto& e : m_ExistingEntities)
		{
			Entity parent = e.second->EntityData.Transform().Parent;
			e.second->Children.resize(0);

			if (parent)
			{
				if (m_EntityParenting[e.second->EntityData] != (uint32_t)parent)
					e.second->IndexInNode = -1;
				m_EntityParenting[e.second->EntityData] = parent;
			}
			else
				m_EntityParenting[e.second->EntityData] = -1;
		}

		// Set parenting 
		for (auto e : m_EntityParenting)
		{
			if (m_EntityParenting[e.first] != -1)
			{
				EntitySceneNode* childNode = m_ExistingEntities[(entt::entity)e.first];
				m_ExistingEntities[(entt::entity)e.second]->Children.push_back(childNode);
				if (childNode->IndexInNode == -1)
					childNode->IndexInNode = m_ExistingEntities[(entt::entity)e.second]->Children.size()-1;
					
			}
		}

		// Order children, assign to scene graph
		for (auto& e : m_ExistingEntities)
		{
			if (!e.second->EntityData.Transform().Parent)
				m_CachedSceneGraph->Children.push_back(e.second);

			if (e.second->IndexInNode == -1)
				e.second->IndexInNode = m_CachedSceneGraph->Children.size() - 1;

			std::sort(e.second->Children.begin(), e.second->Children.end(),
				[&](const EntitySceneNode* right, const EntitySceneNode* left)
				{return right->IndexInNode < left->IndexInNode; });
		}

		std::sort(m_CachedSceneGraph->Children.begin(), m_CachedSceneGraph->Children.end(),
			[&](const EntitySceneNode* right, const EntitySceneNode* left)
			{return right->IndexInNode < left->IndexInNode; });
	}


	void SceneHierarchyPanel::ChangeEntityOrder(uint32_t movedEntity, int position, uint32_t newParentId)
	{
		EntitySceneNode* child = m_ExistingEntities[(entt::entity)movedEntity];
		EntitySceneNode* oldParent = nullptr;
		EntitySceneNode* newParent = nullptr;

		// Load the current parent of the object
		oldParent = m_EntityParenting[movedEntity] == -1 ? 
			nullptr : m_ExistingEntities[(entt::entity)m_EntityParenting[movedEntity]];

		// Load the new parent node
		if (newParentId == -1)
		{
			if (m_ExistingEntities.find((entt::entity)m_EntityParenting[movedEntity]) != m_ExistingEntities.end())
				newParent = m_ExistingEntities[(entt::entity)m_EntityParenting[movedEntity]];
			else
				newParent = m_CachedSceneGraph;
		}
		else
			newParent = m_ExistingEntities[(entt::entity)newParentId];

		// This is to fix indexing issues when deleting a child from a subtree invalidates the other ones
		// on the same one
		int currPos = child->IndexInNode;
		if (position > currPos && m_EntityParenting[movedEntity] == (uint32_t)newParent->EntityData)
			position--;

		// Get subtree children
		std::vector<EntitySceneNode*>& newChildren = newParent->Children;
		std::vector<EntitySceneNode*>& oldChildren = oldParent == nullptr ? m_CachedSceneGraph->Children : oldParent->Children;

		// Remove child from old subtree
		oldChildren.erase(oldChildren.begin() + currPos);
		oldChildren.resize(oldChildren.size());
		uint32_t index = 0;
		for (uint32_t i = 0; i < oldChildren.size(); i++)
			oldChildren[i]->IndexInNode = i;

		// Add child to new subtree
		child->IndexInNode = position;
		newChildren.insert(newChildren.begin() + position, child);
		for (uint32_t i = position+1; i < newChildren.size(); i++)
			newChildren[i]->IndexInNode = newChildren[i - 1]->IndexInNode + 1;

		// Sort children
		std::sort(newChildren.begin(), newChildren.end(),
			[&](const EntitySceneNode* right, const EntitySceneNode* left)
			{return right->IndexInNode < left->IndexInNode; });

		// Save parenting data
		if (newParent == nullptr)
		{
			m_EntityParenting[movedEntity] = -1;
			child->EntityData.Transform().Parent = {};
		}
		else
		{
			m_EntityParenting[movedEntity] = newParent->EntityData;
			child->EntityData.Transform().Parent = newParent->EntityData;
		}

		RebuildSceneGraph();
	}

	void SceneHierarchyPanel::RegisterEntity(const Entity& entity)
	{
		m_ExistingEntities[entity] = new EntitySceneNode(false, entity);
	}

	uint32_t SceneHierarchyPanel::GetParentInSceneGraph(EntitySceneNode* node, uint32_t entity)
	{
		for (uint32_t i = 0; i < node->Children.size(); i++)
			if ((uint32_t)node->Children[i]->EntityData == entity)
				return (uint32_t)node->EntityData;

		for (uint32_t i = 0; i < node->Children.size(); i++)
		{
			bool ret = GetParentInSceneGraph(node->Children[i], entity);
			if (ret)
				return ret;
		}

		return -1;
	}
}