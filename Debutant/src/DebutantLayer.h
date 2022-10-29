#pragma once
#include <Debut.h>
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include "ImGuizmo.h"

#include <Panels/SceneHierarchyPanel.h>
#include <Panels/ContentBrowserPanel.h>
#include <Panels/InspectorPanel.h>
#include <Panels/ViewportPanel.h>
#include <Camera/EditorCamera.h>

namespace Debut
{
	class DebutantLayer : public Layer
	{
		friend class ViewportPanel;

	public:

		DebutantLayer() : Layer("DebutantLayer") {}
		virtual ~DebutantLayer() {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep& ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;

		void OnNewScene();
		void OnOpenScene();
		void OnOpenScene(std::filesystem::path path);
		void OnSaveScene();
		void OnSaveSceneAs();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void LoadModel(const std::filesystem::path path);

	private:
		YAML::Node GetAdditionalSceneData();
	private:
		// UI panels
		void DrawTopBar();
		void DrawSettingsWindow();
		void DrawAssetMapWindow();

		// Debug & Gizmos
		void ManipulatePhysicsGizmos();

		// Drag & droppable objects
		void LoadModelNode(Ref<Model> model, Entity& parent);

	private:
		// Panels
		SceneHierarchyPanel m_SceneHierarchy;
		ContentBrowserPanel m_ContentBrowser;
		PropertiesPanel m_PropertiesPanel;
		InspectorPanel m_Inspector;
		ViewportPanel m_Viewport;

		// Editor state
		Entity m_SelectedEntity;

		bool m_AssetMapOpen = false;
		bool m_SettingsOpen = false;

		uint32_t m_StartIndex = -1;
		uint32_t m_EndIndex = -1;

		// Textures
		AssetCache<std::string, Ref<Texture2D>> m_TextureCache;
		Ref<Scene> m_ActiveScene;
	};
}