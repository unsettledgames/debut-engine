#pragma once

#include <Debut/Core/Core.h>

#include <glm/glm.hpp>
#include <string>
#include <filesystem>

namespace YAML
{
	class Node;
}

namespace Debut
{
	struct Event;
	struct EntitySceneNode;
	
	class  Timestep;
	class Scene;

	class SceneManager
	{
	public:
		enum class SceneState
		{
			Edit = 0, Play = 1
		};

		SceneManager();

		void OnScenePlay();
		void OnSceneStop();

		//void LoadInScene(const std::string& path);

		void NewScene(const glm::vec2& viewportSize);
		EntitySceneNode* OpenScene(const std::filesystem::path& path, YAML::Node& additionalData);
		void SaveScene(const EntitySceneNode& graph, const YAML::Node& additionalData);
		void SaveSceneAs(const EntitySceneNode& graph, const YAML::Node& additionalData);

		inline Ref<Scene> GetActiveScene() { return m_ActiveScene; }
		inline SceneState GetState() { return m_SceneState; }

	private:
		/*
		template <typename T>
		void LoadInScene(Ref<T> toLoad);
		*/

	private:
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		Ref<Scene> m_ActiveScene;

		SceneState m_SceneState = SceneState::Edit;
		std::string m_ScenePath;
	};
}