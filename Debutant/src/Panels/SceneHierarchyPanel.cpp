#include "SceneHierarchyPanel.h"
#include <Debut/Scene/Scene.h>
#include <Debut/Scene/Components.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include "Utils/EditorCache.h"
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Material.h>
#include <Debut/ImGui/ImGuiUtils.h>
#include <entt.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>

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

		Reset();
	}

	void SceneHierarchyPanel::LoadTree(EntitySceneNode* node)
	{
		if (!node->IsRoot)
		{
			Entity parent = node->EntityData.Transform().Parent;
			m_ExistingEntities[node->EntityData] = node;
			if (parent)
				m_EntityParenting[node->EntityData] = parent;
			else
				m_EntityParenting[node->EntityData] = -1;
		}
		else
			m_CachedSceneGraph = node;

		for (uint32_t i = 0; i < node->Children.size(); i++)
			LoadTree(node->Children[i]);
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_HoveringEntity = false;
		m_LastHoveredOpen = false;
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
					{
						// Allow moving an entity above the first one to put it as child of the root node of the scene
						if (parentId == -1 && m_ExistingEntities[(entt::entity)m_LastHoveredEntity]->IndexInNode == 0)
							ChangeEntityOrder(entityId, 0, -1);
						else
							ChangeEntityOrder(entityId, m_ExistingEntities[(entt::entity)m_LastHoveredEntity]->IndexInNode, parentId);
					}
					else
					{
						// Allow moving an entity right under another entity in a different subtree
						if (!m_LastHoveredOpen)
							ChangeEntityOrder(entityId, m_ExistingEntities[(entt::entity)m_LastHoveredEntity]->IndexInNode + 1, parentId);
						else
							ChangeEntityOrder(entityId, m_ExistingEntities[(entt::entity)m_LastHoveredEntity]->Children[0]->IndexInNode, m_LastHoveredEntity);
					}

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
					EntitySceneNode* oldParent = nullptr;
					if (m_EntityParenting[child] != -1)
						oldParent = m_ExistingEntities[(entt::entity)m_EntityParenting[child]];
					else
						oldParent = m_CachedSceneGraph;

					// Shift entity ordering
					if ((uint32_t)oldParent->EntityData != (uint32_t)node.EntityData)
					{
						for (uint32_t i = m_ExistingEntities[(entt::entity)entityId]->IndexInNode + 1; i < oldParent->Children.size(); i++)
							oldParent->Children[i]->IndexInNode = i - 1;
					}

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

		// Keys
		if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) && m_SelectionContext == node.EntityData)
			entityDeleted = true;

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
			if ((uint32_t)m_LastHoveredEntity == (uint32_t)node.EntityData && node.Children.size() > 0)
				m_LastHoveredOpen = true;

			for (uint32_t i = 0; i < node.Children.size(); i++)
				DrawEntityNode(*node.Children[i]);
			ImGui::TreePop();
		}

		m_RebuiltGraph = false;
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
		if (m_CachedSceneGraph)
			delete m_CachedSceneGraph;
		m_CachedSceneGraph = new EntitySceneNode();

		std::unordered_map<uint32_t, bool> existingIds;
		for (auto entity : m_ExistingEntities)
			existingIds[entity.second->EntityData] = true;

		// Add entity entries
		auto transforms = m_Context->m_Registry.view<TransformComponent>();
		// Add entity entries
		for (auto entity : transforms)
		{
			existingIds.erase((uint32_t)entity);

			if (m_ExistingEntities.find(entity) == m_ExistingEntities.end())
				m_ExistingEntities[entity] = new EntitySceneNode(false, { entity, m_Context.get() });
		}

		// Remove entities that don't exist anymore
		for (auto entity : existingIds)
		{
			delete m_ExistingEntities[(entt::entity)entity.first];
			m_ExistingEntities.erase((entt::entity)entity.first);
			m_EntityParenting.erase(entity.first);
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
			newParent = m_CachedSceneGraph;
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
			child->EntityData.Transform().SetParent({});
		}
		else
		{
			m_EntityParenting[movedEntity] = newParent->EntityData;
			child->EntityData.Transform().SetParent(newParent->EntityData);
		}

		RebuildSceneGraph();
	}

	void SceneHierarchyPanel::Reset()
	{
		if (m_CachedSceneGraph)
			delete m_CachedSceneGraph;
		m_CachedSceneGraph = nullptr;

		m_LastHoveredEntity = {};
		m_LastMousePos = {};
		m_LastHoveredMousePos = {};
		m_LastHoveredItemPos = {};
		m_LastItemSize = {};

		m_DroppedOnEntity = false;
		m_DraggingEntity = false;
		m_HoveringEntity = false;
		m_LastHoveredOpen = false;

		for (auto node : m_ExistingEntities)
			delete node.second;

		m_ExistingEntities = {};
		m_EntityParenting = {};
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