#include "DebutantLayer.h"
#include <Debut/Core/Window.h>
#include <Debut/Core/UUID.h>
#include "Debut/Core/Instrumentor.h"
#include <Utils/EditorCache.h>
#include <Debut/Utils/PlatformUtils.h>
#include <Debut/Utils/TransformationUtils.h>
#include <Debut/Utils/CppUtils.h>
#include <Debut/Utils/MathUtils.h>
#include <Camera/EditorCamera.h>
#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Rendering/Resources/Skybox.h>
#include <Debut/Scene/Scene.h>
#include <Debut/Scene/Components.h>
#include <DebutantApp.h>

#include <chrono>
#include "Debut/ImGui/ImGuiUtils.h"
#include <imgui_internal.h>
#include <yaml-cpp/yaml.h>
#include <Debut/Utils/YamlUtils.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

/*
* 
*   CODE REFACTORING:
*       - CURRENT: moving viewport related stuff from DebutantLayer to ViewportPanel
*           - Propagate mouse events to viewport, mouse pick / edit gizmos there too
*       - Group attributes, make them classes or structs. Remove some clutter from DebutantLayer (eg gizmos, viewport data...)
* 
*   QOL:
*       - Drop skybox to set it in the current scene
*       - Shading buttons:
*           - Z buffer
*       
        - Lighting settings:
*           - Use scene lighting
*           - Disable lighting
*           - Editor light: intensity, direction, color
*       
*       - Scene camera settings
* 
    OPTIMIZATION:
        - Remove as many DecomposeTransform as possible
        - Optimize transformation in physics
        - Maybe remove indices from PolygonCollider? The concept is similar to creating a transform matrix every time it's 
          required. Profile both approaches
        - MeshColliders load a whole mesh when only vertices and triangles are needed. Specify flags to know what parts
            to load
        - Materials: Probably time to get rid of YAML and use a binary, compressed format instead

    - Roughness maps (PBR)
    - Reflection maps (PBR)
    
    - Find out why some models are huge or super small sometimes->Don't use model imported transform?
    - Mesh properties in properties panel?
    - Add inspector / properties panel locking
    - Make editor robust to association file deletion / editing
    - Asset renaming
    - Custom memory allocator to keep track of used memory
*/

namespace Debut
{
    void DebutantLayer::OnAttach()
    {
        DBT_PROFILE_SCOPE("DebutantSetup");

        m_ActiveScene = DebutantApp::Get().GetSceneManager().GetActiveScene();
        m_SceneHierarchy.SetContext(m_ActiveScene);
        m_SceneHierarchy.RebuildSceneGraph();

        m_Viewport = ViewportPanel(this);

        m_SceneHierarchy.SetInspectorPanel(&m_Inspector);
        m_ContentBrowser.SetPropertiesPanel(&m_PropertiesPanel);

        AssetManager::Init(".");
    }

    void DebutantLayer::OnDetach()
    {

    }

    void DebutantLayer::OnUpdate(Timestep& ts)
    {
        m_Viewport.OnUpdate(ts);
    }

    void DebutantLayer::OnImGuiRender()
    {
        DBT_PROFILE_FUNCTION();

        auto stats = Renderer2D::GetStats();

        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
            if (!opt_padding)
                ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            ImGuiStyle& style = ImGui::GetStyle();

            style.WindowMinSize.x = 350.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            DrawTopBar();

            {
                DBT_PROFILE_SCOPE("Debutant::HierarchyUpdate");
                m_SceneHierarchy.OnImGuiRender();
            }
            {
                DBT_PROFILE_SCOPE("Debutant::InspectorUpdate");
                m_Inspector.OnImGuiRender();
            }
            {
                DBT_PROFILE_SCOPE("Debutant::ContentBrowserUpdate");
                m_ContentBrowser.OnImGuiRender();
            }
            {
                DBT_PROFILE_SCOPE("Debutant::PropertiesPanelUpdate");
                m_PropertiesPanel.OnImGuiRender();
            }
            {
                DBT_PROFILE_SCOPE("Debutant::ViewportPanelUpdate");
                m_Viewport.OnImGuiRender();
            }

            // Entity changed from scene hierarchy
            if (m_SceneHierarchy.GetSelectionContext() != m_SelectedEntity)
            {
                m_SelectedEntity = m_SceneHierarchy.GetSelectionContext();
                m_Viewport.SetSelectedEntity(m_SceneHierarchy.GetSelectionContext());
                m_Inspector.SetSelectedEntity(m_SceneHierarchy.GetSelectionContext());
            }

#ifdef DBT_DEBUG
            if (m_AssetMapOpen)
                DrawAssetMapWindow();
#endif
            if (m_SettingsOpen)
                DrawSettingsWindow();

        ImGui::End();
    }

    void DebutantLayer::DrawSettingsWindow()
    {
        static std::unordered_set<std::string> changedSettings;
        ImGuiWindowFlags flags = ImGuiWindowFlags_Modal;
        
        ImGui::Begin("Settings", &m_SettingsOpen, flags);
        ImGui::SetWindowSize({ 800, 600 });

        if (ImGui::BeginTabBar("SettingsTabBar"))
        {
            if (ImGui::BeginTabItem("Editor"))
            {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Graphics"))
            {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Lighting"))
            {
                // Skybox options
                ImGuiUtils::BoldText("Skybox");
                Ref<Skybox> currSkybox = m_ActiveScene->GetSkybox();
                Debut::UUID skybox = ImGuiUtils::DragDestination("Skybox", ".skybox", currSkybox == nullptr ? 0 : currSkybox->GetID());

                if (m_ActiveScene->GetSkybox() == nullptr && skybox != 0)
                    m_ActiveScene->SetSkybox(skybox);
                else if (skybox != 0 &&m_ActiveScene->GetSkybox()->GetID() != skybox)
                    m_ActiveScene->SetSkybox(skybox);

                // Ambient light
                glm::vec3 ambientLight = m_ActiveScene->GetAmbientLight();
                ImGuiUtils::Color3("Ambient light", { &ambientLight.r, &ambientLight.g, &ambientLight.b });
                m_ActiveScene->SetAmbientLight(ambientLight);
                // Ambient light intensity
                float ambientIntensity = m_ActiveScene->GetAmbientLightIntensity();
                if (ImGuiUtils::DragFloat("Ambient light intensity", &ambientIntensity, 0.1f, 0.0))
                    m_ActiveScene->SetAmbientLightIntensity(ambientIntensity);

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Physics"))
            {
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        if (ImGui::Button("Apply settings"))
        {
            // Apply all settings
            for (auto& setting : changedSettings)
            {
            }
            // Reset the set
            changedSettings = {};
        }

        ImGui::End();
    }

    void DebutantLayer::DrawAssetMapWindow()
    {
        const auto& assetMap = AssetManager::GetAssetMap();
        uint32_t i = 0;
        ImGui::Begin("Asset map", &m_AssetMapOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Modal);

        for (auto& entry : assetMap)
        {
            std::stringstream ss;
            ss << entry.first << ":\t\t  " << entry.second;
            bool selected = (m_StartIndex != -1 && m_EndIndex != -1 && (i > m_StartIndex && i <= m_EndIndex)) || i == m_StartIndex;
            
            if (ImGui::Selectable(ss.str().c_str(), &selected, ImGuiSelectableFlags_SpanAvailWidth))
            {
                Log.CoreInfo("Shift clicked: {0}", Input::IsKeyPressed(DBT_KEY_LEFT_SHIFT) || Input::IsKeyPressed(DBT_KEY_RIGHT_SHIFT));
                if (ImGui::IsKeyPressed(ImGuiKey_LeftShift) || ImGui::IsKeyPressed(ImGuiKey_RightShift))
                    m_EndIndex = i;
                else
                {
                    m_StartIndex = i;
                    m_EndIndex = -1;
                }
            }

            ImGui::Separator();
            i++;
        }

        ImGui::End();

        if (m_StartIndex > m_EndIndex)
        {
            uint32_t tmp = m_StartIndex;
            m_StartIndex = m_EndIndex;
            m_EndIndex = tmp;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            std::vector<Debut::UUID> toDel;

            for (uint32_t i = m_StartIndex; i < m_EndIndex; i++)
                toDel.push_back(assetMap[i].first);

            AssetManager::DeleteAssociations(toDel);

            m_StartIndex = -1;
            m_EndIndex = -1;
        }
    }

    void DebutantLayer::DrawTopBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New scene", "Ctrl+N"))
                    OnNewScene();

                if (ImGui::MenuItem("Open scene", "Ctrl+O"))
                    OnOpenScene();

                if (ImGui::MenuItem("Save scene", "Ctrl+S"))
                    OnSaveScene();

                if (ImGui::MenuItem("Save scene as...", "Ctrl+Shift+S"))
                    OnSaveSceneAs();

                if (ImGui::MenuItem("Exit")) 
                    Application::Get().Close();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets"))
            {
                if (ImGui::MenuItem("Reimport"))
                    AssetManager::Reimport();
                if (ImGui::MenuItem("Asset map"))
                    m_AssetMapOpen = true;
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Project"))
            {
                if (ImGui::MenuItem("Settings"))
                    m_SettingsOpen = true;

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }


    void DebutantLayer::LoadModel(const std::filesystem::path path)
    {
        Ref<Model> model = AssetManager::Request<Model>(path.string());
        Entity modelEntity = m_ActiveScene->CreateEntity({}, path.filename().string());

        modelEntity.Transform().SetParent({});
        LoadModelNode(model, modelEntity);

        m_SceneHierarchy.RebuildSceneGraph();
    }
    void DebutantLayer::LoadModelNode(Ref<Model> model, Entity& parent)
    {
        std::string path, extension, folder, name;
        path = model->GetPath();
        if (path != "")
        {
            extension = path.substr(path.find_last_of("."), path.length());
            folder = path.substr(path.find_last_of("\\"), path.length());
            name = folder.substr(folder.find_last_of("\\") + 1, folder.length() - extension.length());
        }
        else
            name = "Submodel";

        // Create entity
        Entity modelEntity = m_ActiveScene->CreateEntity({}, name);
        // Register the entity in the hierarchy

        // Parent it
        modelEntity.Transform().SetParent(parent);

        // Add MeshRendererComponent: if there are more than 1 mesh, add children
        if (model->GetMeshes().size() == 1)
            modelEntity.AddComponent<MeshRendererComponent>(model->GetMeshes()[0], model->GetMaterials()[0]);
        else
        {
            for (uint32_t i = 0; i < model->GetMeshes().size(); i++)
            {
                Entity additional = m_ActiveScene->CreateEntity({}, name + " i");
                additional.Transform().SetParent(modelEntity);
                additional.AddComponent<MeshRendererComponent>(model->GetMeshes()[i], model->GetMaterials()[i]);
            }
        }

        // Add submodels as children
        for (uint32_t i = 0; i < model->GetSubmodels().size(); i++)
            LoadModelNode(AssetManager::Request<Model>(model->GetSubmodels()[i]), modelEntity);
    }

    void DebutantLayer::ManipulatePhysicsGizmos()
    {
        /*
        DBT_PROFILE_FUNCTION();
        Entity currSelection = m_SceneHierarchy.GetSelectionContext();

        if (currSelection)
        {
            // Convert from collider space to world space
            glm::mat4 pointTransform = glm::translate(m_PhysicsSelection.PointTransform, m_PhysicsSelection.SelectedPoint);

            // Manipulate the selected point
            if (ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetView()),
                glm::value_ptr(m_EditorCamera.GetProjection()), ImGuizmo::TRANSLATE,
                m_GizmoMode, glm::value_ptr(pointTransform)))
            {
                // Apply the changes
                glm::vec3 newPoint = m_PhysicsSelection.SelectedPoint;

                glm::vec3 finalTrans, finalRot, finalScale;
                MathUtils::DecomposeTransform(pointTransform, finalTrans, finalRot, finalScale);
                // Convert back to collider space
                newPoint = glm::vec3(glm::inverse(m_PhysicsSelection.PointTransform) * glm::vec4(finalTrans, 1.0f));

                // Send the changes to the collider
                if (newPoint != m_PhysicsSelection.SelectedPoint)
                {
                    if (currSelection.HasComponent<BoxCollider2DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateBoxCollider2D");

                        BoxCollider2DComponent& boxCollider = currSelection.GetComponent<BoxCollider2DComponent>();
                        boxCollider.SetPoint(glm::vec2(newPoint), m_PhysicsSelection.SelectedName);
                    }
                    else if (currSelection.HasComponent<CircleCollider2DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateCircleCollider2D");

                        CircleCollider2DComponent& circleCollider = currSelection.GetComponent<CircleCollider2DComponent>();
                        if (m_PhysicsSelection.SelectedName == "Top" || m_PhysicsSelection.SelectedName == "Bottom")
                            newPoint.x = circleCollider.Offset.x;
                        else
                            newPoint.y = circleCollider.Offset.y;

                        circleCollider.SetPoint(glm::vec2(newPoint.x, newPoint.y), m_PhysicsSelection.SelectedName);
                    }
                    else if (currSelection.HasComponent<PolygonCollider2DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdatePolygonCollider2D");

                        PolygonCollider2DComponent& polygonCollider = currSelection.GetComponent<PolygonCollider2DComponent>();
                        polygonCollider.SetPoint(std::stoi(m_PhysicsSelection.SelectedName), glm::vec2(newPoint));
                    }
                    else if (currSelection.HasComponent<BoxCollider3DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateBoxCollider3D");

                        BoxCollider3DComponent& boxCollider = currSelection.GetComponent<BoxCollider3DComponent>();
                        boxCollider.SetPoint(m_PhysicsSelection.SelectedName, newPoint);
                    }
                    else if (currSelection.HasComponent<SphereCollider3DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateSphereCollider3D");

                        SphereCollider3DComponent& sphereCollider = currSelection.GetComponent<SphereCollider3DComponent>();
                        sphereCollider.SetPoint(m_PhysicsSelection.SelectedName, newPoint);
                    }
                }

                m_PhysicsSelection.SelectedPoint = newPoint;
            }
        }
        */
    }

    void DebutantLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        if (DebutantApp::Get().GetSceneManager().GetState() == SceneManager::SceneState::Edit)
            m_Viewport.Camera().OnEvent(e);

        dispatcher.Dispatch<KeyPressedEvent>(DBT_BIND(DebutantLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(DBT_BIND(DebutantLayer::OnMouseButtonPressed));
    }

    bool DebutantLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        switch (e.GetKeyCode())
        {
        // Hierarchy shortcuts
        case DBT_KEY_D:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                m_ActiveScene->DuplicateEntity(m_SceneHierarchy.GetSelectionContext(), m_SceneHierarchy.GetSelectionContext().Transform().Parent);
            break;
        // File Menu
        case DBT_KEY_N:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                OnNewScene();
            break;
        case DBT_KEY_O:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                OnOpenScene();
            break;
        case DBT_KEY_S:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                if (Input::IsKeyPressed(DBT_KEY_LEFT_SHIFT) || Input::IsKeyPressed(DBT_KEY_RIGHT_SHIFT))
                    OnSaveSceneAs();
                else
                    OnSaveScene();
            break;
        default:
            m_Viewport.OnKeyPressed(e);
            break;
        }

        return true;
    }

    bool DebutantLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        Ref<Scene> activeScene = DebutantApp::Get().GetSceneManager().GetActiveScene();

        // Mouse picking
        if (e.GetMouseButton() == DBT_MOUSE_BUTTON_LEFT && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
        {
            glm::vec2 mouseCoords = m_Viewport.GetFrameBufferCoords();
            int col = m_Viewport.GetFrameBuffer()->ReadRedPixel(1, mouseCoords.x, mouseCoords.y);

            if (col < 0)
                m_SelectedEntity = {};
            else
            {
                Entity entity = { (entt::entity)col, activeScene.get()};
                if (entity.IsValid())
                    m_SelectedEntity = entity;
                else
                    m_SelectedEntity = {};

                m_SceneHierarchy.SetSelectedEntity(m_SelectedEntity);
                m_Inspector.SetSelectedEntity(m_SelectedEntity);
                m_Viewport.SetSelectedEntity(m_SelectedEntity);
            }
        }

        return true;
    }

    void DebutantLayer::OnNewScene()
    {
        glm::vec2 viewportSize = m_Viewport.GetViewportSize();
        DebutantApp::Get().GetSceneManager().NewScene(viewportSize);

        m_SceneHierarchy.SetContext(m_ActiveScene);
        m_SceneHierarchy.RebuildSceneGraph();
    }

    void DebutantLayer::OnOpenScene()
    {
        std::string path = FileDialogs::OpenFile("Debut Scene (*.debut)\0*.debut\0");
        if (!path.empty())
            OnOpenScene(path);
    }

    void DebutantLayer::OnOpenScene(std::filesystem::path path)
    {
        YAML::Node additionalData;

        SceneManager& sceneManager = DebutantApp::Get().GetSceneManager();
        EntitySceneNode* sceneNode = sceneManager.OpenScene(path, additionalData);

        m_SceneHierarchy.SetContext(sceneManager.GetActiveScene());
        m_SceneHierarchy.LoadTree(sceneNode);
        m_SceneHierarchy.RebuildSceneGraph();

        if (additionalData["Valid"].as<bool>())
        {
            if (additionalData["EditorCameraPitch"])
                m_Viewport.Camera().SetPitch(additionalData["EditorCameraPitch"].as<float>());
            if (additionalData["EditorCameraYaw"])
                m_Viewport.Camera().SetYaw(additionalData["EditorCameraYaw"].as<float>());
            if (additionalData["EditorCameraFocalPoint"])
                m_Viewport.Camera().SetFocalPoint(additionalData["EditorCameraFocalPoint"].as<glm::vec3>());
            if (additionalData["EditorCameraDistance"])
                m_Viewport.Camera().SetDistance(additionalData["EditorCameraDistance"].as<float>());
        }
    }

    void DebutantLayer::OnSaveScene()
    {
        DebutantApp::Get().GetSceneManager().SaveScene(*m_SceneHierarchy.GetSceneGraph(), GetAdditionalSceneData());
    }

    void DebutantLayer::OnSaveSceneAs()
    {
        DebutantApp::Get().GetSceneManager().SaveSceneAs(*m_SceneHierarchy.GetSceneGraph(), GetAdditionalSceneData());
    }

    YAML::Node DebutantLayer::GetAdditionalSceneData()
    {
        YAML::Node additionalData;
        additionalData["EditorCameraPitch"] = m_Viewport.Camera().GetPitch();
        additionalData["EditorCameraYaw"] = m_Viewport.Camera().GetYaw();
        additionalData["EditorCameraFocalPoint"] = m_Viewport.Camera().GetFocalPoint();
        additionalData["EditorCameraDistance"] = m_Viewport.Camera().GetDistance();

        return additionalData;
    }
}
