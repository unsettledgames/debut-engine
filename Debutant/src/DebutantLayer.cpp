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
*   CURRENT: OPTIMIZATION (~200K triangles)
*       Start:                  ~58 FPS -> 17.241 ms
*       Matrix opt:             ~60 FPS -> 16.666 ms
*       Frustum culling:        ~75 FPS -> 13.333 ms
*       No reupload:            ~120 FPS-> 9.3333 ms
*       Shader optimization:    ~170 FPS-> 5.8823 ms (disabled battery saving tho :P, otherwise 130 FPS and 7.692 ms)
* 
        - OpenGL optimizations: https://on-demand.gputechconf.com/siggraph/2014/presentation/SG4117-OpenGL-Scene-Rendering-Techniques.pdf
        - Scene graph optimizations: https://on-demand.gputechconf.com/gtc/2013/presentations/S3032-Advanced-Scenegraph-Rendering-Pipeline.pdf
*
*   MAIN SHADOW WORKFLOW
* 
*   - Point shadow mapping
*       - Well, start by implementing the base workflow with a single light. Cubemaps are needed, which means 6 * shadowMapRes
*           pixels of stuff per light. Doom 2016 apparently uses an 8K shadow map atlas, which means you only have to bind
*           a single texture and then pass the right UVs. 8000x8000 leaves room for 64 1000x1000 shadow maps, which is pretty
*           nice? See optimization section about that
* 
*   OPTIMIZATION AND IMPROVEMENTS
*   - Configurable PCSS for soft shadows
*   - Important lights: find the lights that, at the moment, are important. The nearest to the camera? Always consider the 
*       directional light(s?), I wonder if there's some cheap way to check if the shadows produced by a light will be visible 
*       in the scnee without actually rendering the scene. I don't think so.
*   - A fast approach to the above issue would consist in reserving higher resolution maps to important lights and smaller
*       maps to ones that aren't. Also keep in mind that what Doom 2016 is a very specific game with very specific needs, their
*       approach might not be optimal for a generic use. What if multiple shadow maps depending on level of importance?
*   -  Gaussian blur on the shadow maps?
* 
*   QOL:
*       - Find a better way to update entity selection: if it's selected or destroyed somewhere, it must be selected 
*           or destroyed everywhere
*       - Highlight selected entity in scene hierarchy when it's selected in the viewport
*       - Render Light directions and gizmos, same for camera
*       - Move Texture, Shader, Material and SubTexture2D to Resources folder
*       - Change way of rendering DepthMap (use Depth mode of RenderTexture)
*       - Custom events, propagated starting from the Application: in this way we can avoid pointers to other classes
*           (e.g. DebutantLayer* in ViewportPanel, which I really don't like at the moment)
*       - Drop skybox to set it in the current scene
*       
        - Lighting settings:
*           - Use scene lighting
*           - Disable lighting
*           - Editor light: intensity, direction, color
*       
*       - Scene camera settings
*           - Movement data
*       - The debug renderer should probably only used in a DebugLayer since it kinda behaves as such
*       - Implement rendering modes in 2D too
*       - Render camera frustum
* 
    OPTIMIZATION:
*       - Update AssetMap only OnClose or once when this layer is attached
        - Remove as many DecomposeTransform as possible
        - Optimize transformation in physics
        - Maybe remove indices from PolygonCollider? The concept is similar to creating a transform matrix every time it's 
          required. Profile both approaches
        - MeshColliders load a whole mesh when only vertices and triangles are needed. Specify flags to know what parts
            to load
        - Materials: Probably time to get rid of YAML and use a binary, compressed format instead

    - Roughness maps (PBR)
    - Reflection maps (PBR)
    
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
        m_ActiveScene = DebutantApp::Get().GetSceneManager().GetActiveScene();
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
            if (m_SceneHierarchy.GetSelectionContext() != m_SelectedEntity || m_Viewport.GetSelectedEntity() != m_SelectedEntity)
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

        m_ActiveScene = DebutantApp::Get().GetSceneManager().GetActiveScene();

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
