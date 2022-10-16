#pragma once

#include <imgui.h>
#include <vector>
#include <Debut/Core/Core.h>
#include <Debut/Scene/Entity.h>

namespace Debut
{
	class Scene;

	struct TransformComponent;

	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetContext(const Ref<Scene>& scene);
		void SetSelectedEntity(const Entity& entity);
		void RegisterEntity(const Entity& entity);

		void OnImGuiRender();

		inline Entity GetSelectionContext() { 
			if (m_SelectionContext)
			{
				TransformComponent& transform = m_SelectionContext.Transform();
				int sas = 5;
			}
			return m_SelectionContext; 
		}

		inline EntitySceneNode* GetSceneGraph() { return m_CachedSceneGraph; }

		void RebuildSceneGraph();
		void Reset();
		void LoadTree(EntitySceneNode* node);
		void ChangeEntityOrder(uint32_t movedEntity, int position, uint32_t newParent = -1);

	private:
		uint32_t GetParentInSceneGraph(EntitySceneNode* node, uint32_t entity);

		void DrawEntityNode(EntitySceneNode& entity);
		void DrawComponents(Entity& entity);

		template <typename T>
		bool DrawAddReplaceComponentMenu(bool replace = false)
		{
			bool ret = false;

			if (ImGui::BeginPopup("AddReplaceComponent"))
			{
				if (ImGui::BeginMenu("Rendering"))
				{
					ret |= DrawAddReplaceComponentEntry<CameraComponent, T>("Camera", replace);
					ret |= DrawAddReplaceComponentEntry<SpriteRendererComponent, T>("Sprite Renderer", replace);
					ret |= DrawAddReplaceComponentEntry<MeshRendererComponent, T>("Mesh Renderer", replace);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Physics and Colliders"))
				{
					// Rigidbodies
					ret |= DrawAddReplaceComponentEntry<Rigidbody2DComponent, T>("Rigidbody2D", replace);
					ret |= DrawAddReplaceComponentEntry<Rigidbody3DComponent, T>("Rigidbody3D", replace);

					ImGuiUtils::Separator();

					// 2D Colliders
					ret |= DrawAddReplaceComponentEntry<BoxCollider2DComponent, T>("Box Collider 2D", replace);
					ret |= DrawAddReplaceComponentEntry<CircleCollider2DComponent, T>("Circle Collider 2D", replace);
					ret |= DrawAddReplaceComponentEntry<PolygonCollider2DComponent, T>("Polygon Collider", replace);

					ImGuiUtils::Separator();

					// 3D Colliders
					ret |= DrawAddReplaceComponentEntry<BoxCollider3DComponent, T>("Box Collider 3D", replace);
					ret |= DrawAddReplaceComponentEntry<SphereCollider3DComponent, T>("Sphere Collider 3D", replace);
					ret |= DrawAddReplaceComponentEntry<MeshCollider3DComponent, T>("Mesh Collider 3D", replace);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Lighting"))
				{
					ret |= DrawAddReplaceComponentEntry<DirectionalLightComponent, T>("Directional Light", replace);
					ret |= DrawAddReplaceComponentEntry<PointLightComponent, T>("Point Light", replace);

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			return ret;
		}

		template <typename N, typename O>
		bool DrawAddReplaceComponentEntry(const std::string& componentName, bool replace = false)
		{
			if (replace)
				return DrawReplaceComponentEntry<N, O>(componentName);
			else
				return DrawAddComponentEntry<N>(componentName);
		}

		template <typename T>
		bool DrawAddComponentEntry(const std::string& componentName)
		{
			if (!m_SelectionContext.HasComponent<T>())
			{
				if (ImGui::MenuItem(componentName.c_str()))
				{
					m_SelectionContext.AddComponent<T>();
					ImGui::CloseCurrentPopup();
					return true;
				}
			}

			return false;
		}

		template <typename N, typename O>
		bool DrawReplaceComponentEntry(const std::string& componentName)
		{
			if (!m_SelectionContext.HasComponent<N>())
			{
				if (ImGui::MenuItem(componentName.c_str()))
				{
					m_SelectionContext.AddComponent<N>();
					m_SelectionContext.RemoveComponent<O>();
					ImGui::CloseCurrentPopup();
					return true;
				}
			}

			return false;
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
			bool draw = true;
			ImGui::PopStyleVar();

			ImGui::SameLine(availRegion.x - lineHeight * 0.5f);
			if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
				ImGui::OpenPopup("ComponentSettings");

			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					remove = true;
				else if (name != "Transform" && ImGui::BeginMenu("Replace component"))
				{
					ImGui::OpenPopup("AddReplaceComponent");
					draw = !DrawAddReplaceComponentMenu<T>(true);
					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			if (draw)
			{
				auto& component = entity.GetComponent<T>();
				uiFunction(component);
			}
			if (open)
				ImGui::TreePop();

			if (remove)
				entity.RemoveComponent<T>();
		}

		void DestroySceneNode(EntitySceneNode& node);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

		// Scene graph management
		bool m_RebuiltGraph = false;
		EntitySceneNode* m_CachedSceneGraph;
		std::unordered_map<entt::entity, EntitySceneNode*> m_ExistingEntities;
		// Key is child of value
		std::unordered_map<uint32_t, uint32_t> m_EntityParenting;

		// State
		Entity m_LastHoveredEntity = {};
		ImVec2 m_LastMousePos = {};
		ImVec2 m_LastHoveredMousePos = {};
		ImVec2 m_LastHoveredItemPos = {};
		ImVec2 m_LastItemSize = {};

		bool m_DroppedOnEntity = false;
		bool m_DraggingEntity = false;
		bool m_HoveringEntity = false;
		bool m_LastHoveredOpen = false;
	};
}
