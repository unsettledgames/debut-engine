#include <Scene/SceneManager.h>

#include <Debut/Scene/Entity.h>
#include <Debut/Scene/Components.h>
#include <Debut/Scene/SceneSerializer.h>

#include <Debut/Utils/PlatformUtils.h>
#include <Debut/Utils/CppUtils.h>

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace Debut
{
    SceneManager::SceneManager()
    {
        NewScene({ 1600, 900 });
    }

    void SceneManager::OnScenePlay()
    {
        m_SceneState = SceneState::Play;

        m_RuntimeScene = Scene::Copy(m_ActiveScene);
        m_RuntimeScene->OnRuntimeStart();

        m_ActiveScene = m_RuntimeScene;
    }

    void SceneManager::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
        m_ActiveScene->OnRuntimeStop();

        m_RuntimeScene = nullptr;
        m_ActiveScene = m_EditorScene;
    }

    void SceneManager::NewScene(const glm::vec2& viewportSize)
    {
        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        m_EditorScene = CreateRef<Scene>();
        m_EditorScene->OnViewportResize(viewportSize.x, viewportSize.y);

        Entity camera = m_EditorScene->CreateEntity({}, "Camera");
        CameraComponent& cameraComp = camera.AddComponent<CameraComponent>();
        cameraComp.Camera.SetPerspective(30, 0.1f, 1000.0f);
        cameraComp.Camera.SetFOV(glm::radians(40.0f));
        camera.Transform().Translation = glm::vec3(0.0f, 5.0f, 10.0f);

        Entity directionalLight = m_EditorScene->CreateEntity({}, "Directional light");
        DirectionalLightComponent& light = directionalLight.AddComponent<DirectionalLightComponent>();
        light.Direction = glm::vec3(0.5f, 0.5f, 0.5f);

        m_ActiveScene = m_EditorScene;
        m_RuntimeScene = nullptr;
        m_ScenePath = "";
    }

    EntitySceneNode* SceneManager::OpenScene(const std::filesystem::path& path, YAML::Node& additionalData)
    {
        additionalData["Valid"] = false;

        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        m_RuntimeScene = nullptr;
        Ref<Scene> oldScene = m_EditorScene;
        m_EditorScene = CreateRef<Scene>();
        m_EditorScene->OnViewportResize(oldScene->GetViewportSize().x, oldScene->GetViewportSize().y);

        SceneSerializer ss(m_EditorScene);
        EntitySceneNode* sceneHierarchy = ss.DeserializeText(path.string(), additionalData);

        m_ScenePath = path.string();
        m_ActiveScene = m_EditorScene;

        return sceneHierarchy;
    }

    void SceneManager::SaveScene(const EntitySceneNode& graph, const YAML::Node& additionalData)
    {
        if (m_ScenePath == "")
        {
            SaveSceneAs(graph, additionalData);
            return;
        }

        SceneSerializer ss(m_ActiveScene);
        ss.SerializeText(m_ScenePath, graph, additionalData);
    }

    void SceneManager::SaveSceneAs(const EntitySceneNode& graph, const YAML::Node& additionalData)
    {
        std::string path = FileDialogs::SaveFile("Debut Scene (*.debut)\0*.debut\0");
        if (!path.empty())
        {
            if (!CppUtils::String::EndsWith(path, ".debut"))
                path += ".debut";
            m_ScenePath = path;
            SaveScene(graph, additionalData);
        }
    }

    /*void SceneManager::LoadInScene(const std::string& path)
    {
        if (CppUtils::String::EndsWith(path, ".debut"))
        {

        }
    }*/
}