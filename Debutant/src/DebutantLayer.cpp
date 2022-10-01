#include "DebutantLayer.h"
#include <Debut/Core/Window.h>
#include <Debut/Core/UUID.h>
#include "Debut/Core/Instrumentor.h"
#include <Utils/EditorCache.h>
#include <Debut/Utils/PlatformUtils.h>
#include <Debut/Utils/TransformationUtils.h>
#include <Debut/Utils/CppUtils.h>
#include <Camera/EditorCamera.h>
#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Rendering/Resources/Skybox.h>

#include <chrono>
#include "Debut/ImGui/ImGuiUtils.h"
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

/*
*   CURRENT: separate physics gizmos rendering from actual manipulation
* 
    TODO:
    - 2 main things!
        - Collider gizmos
            - BoxCollider: drag vertices to edit
            - CircleCollider: drag one of the 4 points to expand

            In general
                - Right click deletes a vertex, if appliable
                - Editing happens in 3D space, but the effects should be limited to 2D space, in which the up vector 
                  is the normal of the quad
        - Polygon Collider
            - Gizmos: drag center to move, drag vertices to edit


    - Roughness maps (PBR)
    - Reflection maps (PBR)
    
    - Find out why some models are huge or super small sometimes
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
        FrameBufferSpecifications fbSpecs;
        fbSpecs.Attachments = { 
            FrameBufferTextureFormat::Color, FrameBufferTextureFormat::Depth,
            FrameBufferTextureFormat::RED_INTEGER
        };

        fbSpecs.Width = Application::Get().GetWindow().GetWidth();
        fbSpecs.Height = Application::Get().GetWindow().GetHeight();

        m_FrameBuffer = FrameBuffer::Create(fbSpecs);

        m_EditorScene = CreateRef<Scene>();
        m_ActiveScene = m_EditorScene;
        m_EditorCamera = EditorCamera(30, 16.0f / 9.0f, 0.1f, 1000.0f);

        m_SceneHierarchy.SetContext(m_ActiveScene);
        m_SceneHierarchy.RebuildSceneGraph();
        m_ContentBrowser.SetPropertiesPanel(&m_PropertiesPanel);

        m_IconPlay = Texture2D::Create("assets\\icons\\play.png");
        m_IconStop = Texture2D::Create("assets\\icons\\stop.png");

        AssetManager::Init(".");

        EditorCache::Textures().Put("assets\\icons\\play.png", m_IconPlay);
        EditorCache::Textures().Put("assets\\icons\\stop.png", m_IconStop);

        // TEST AREA
        std::vector<std::string> filenames = { "assets\\textures\\Skybox\\front.png" ,
        "assets\\textures\\Skybox\\bottom.png","assets\\textures\\Skybox\\left.png",
        "assets\\textures\\Skybox\\right.png","assets\\textures\\Skybox\\up.png",
        "assets\\textures\\Skybox\\down.png" };

        //m_ActiveScene->SetSkybox(12755579250371982529);
    }

    void DebutantLayer::OnDetach()
    {

    }

    void DebutantLayer::OnUpdate(Timestep& ts)
    {
        DBT_PROFILE_FUNCTION();
        //Log.CoreInfo("FPS: {0}", 1.0f / ts);
        // Update camera
        if (m_ViewportFocused)
            m_EditorCamera.OnUpdate(ts);

        Renderer2D::ResetStats();
        {
            DBT_PROFILE_SCOPE("Debutant::RendererSetup");
            m_FrameBuffer->Bind();

            RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
            RenderCommand::Clear();

            // Clear frame buffer for mouse picking
            m_FrameBuffer->ClearAttachment(1, -1);
        }

        // Update the scene
        switch (m_SceneState)
        {
        case SceneState::Play:
            m_ActiveScene->OnRuntimeUpdate(ts);
            break;
        case SceneState::Edit:
            m_ActiveScene->OnEditorUpdate(ts, m_EditorCamera);
            // Render debug
            DrawPhysicsGizmos();
            break;
        }

        m_FrameBuffer->Unbind();
    }

    void DebutantLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Play;

        m_RuntimeScene = Scene::Copy(m_ActiveScene);
        m_RuntimeScene->OnRuntimeStart();

        m_ActiveScene = m_RuntimeScene;
    }

    void DebutantLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
        m_ActiveScene->OnRuntimeStop();

        m_RuntimeScene = nullptr;
        m_ActiveScene = m_EditorScene;
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

            m_SceneHierarchy.OnImGuiRender();
            m_ContentBrowser.OnImGuiRender();
            m_PropertiesPanel.OnImGuiRender();

#ifdef DBT_DEBUG
            if (m_AssetMapOpen)
                DrawAssetMapWindow();
#endif
            if (m_SettingsOpen)
                DrawSettingsWindow();

            DrawViewport();
            DrawUIToolbar();

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

    // TODO: move this to the top bar
    void DebutantLayer::DrawUIToolbar()
    {
        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        Ref<Texture2D> icon;
        float buttonSize = ImGui::GetWindowHeight() - 20.0f;

        switch (m_SceneState)
        {
        case SceneState::Edit:
            icon = EditorCache::Textures().Get("assets\\icons\\play.png");
            break;
        case SceneState::Play:
            icon = EditorCache::Textures().Get("assets\\icons\\stop.png");
            break;
        default:
            break;
        }

        ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.5f) - (buttonSize * 0.5f));
        if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(buttonSize, buttonSize)))
        {
            // TODO: pause scene...
            if (m_SceneState == SceneState::Edit)
                OnScenePlay();
            else if (m_SceneState == SceneState::Play)
                OnSceneStop();
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
                    NewScene();

                if (ImGui::MenuItem("Open scene", "Ctrl+O"))
                    OpenScene();

                if (ImGui::MenuItem("Save scene", "Ctrl+S"))
                    SaveScene();

                if (ImGui::MenuItem("Save scene as...", "Ctrl+Shift+S"))
                    SaveSceneAs();

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

    void DebutantLayer::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");
        {
            // Window resizing
            auto viewportOffset = ImGui::GetCursorPos();

            m_ViewportFocused = ImGui::IsWindowFocused();
            m_ViewportHovered = ImGui::IsWindowHovered();
            Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            if (m_ViewportSize.x != viewportSize.x || m_ViewportSize.y != viewportSize.y)
            {
                m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);

                m_FrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);

                m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
                m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            }

            // Draw scene
            uint32_t texId = m_FrameBuffer->GetColorAttachment();
            ImGui::Image((void*)texId, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2{ 0,1 }, ImVec2{ 1,0 });

            // Accept scene loading
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA");
                if (payload != nullptr)
                {
                    std::filesystem::path path((const wchar_t*)payload->Data);
                    if (path.extension() == ".debut")
                        OpenScene(path);
                    else if (path.extension() == ".model")
                        LoadModel(path);
                    ImGui::EndDragDropTarget();
                }

            }

            // Save bounds for mouse picking
            ImVec2 minBound = ImGui::GetItemRectMin();
            ImVec2 maxBound = ImGui::GetItemRectMax();
            m_ViewportBounds[0] = { minBound.x, minBound.y };
            m_ViewportBounds[1] = { maxBound.x, maxBound.y };

            if (m_SceneState == SceneState::Edit)
                DrawTransformGizmos();

            ImGui::PopStyleVar();
        }
        ImGui::End();
    }

    void DebutantLayer::LoadModel(const std::filesystem::path path)
    {
        Ref<Model> model = AssetManager::Request<Model>(path.string());
        Entity modelEntity = m_ActiveScene->CreateEntity({}, path.filename().string());
        m_SceneHierarchy.RegisterEntity(modelEntity);

        modelEntity.Transform().Parent = {};
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
        m_SceneHierarchy.RegisterEntity(modelEntity);

        // Parent it
        modelEntity.Transform().Parent = parent;

        // Add MeshRendererComponent: if there are more than 1 mesh, add children
        if (model->GetMeshes().size() == 1)
            modelEntity.AddComponent<MeshRendererComponent>(model->GetMeshes()[0], model->GetMaterials()[0]);
        else
            for (uint32_t i = 0; i < model->GetMeshes().size(); i++)
            {
                Entity additional = m_ActiveScene->CreateEntity({}, name + " i");
                additional.Transform().Parent = modelEntity;
                additional.AddComponent<MeshRendererComponent>(model->GetMeshes()[i], model->GetMaterials()[i]);
                m_SceneHierarchy.RegisterEntity(additional);
            }

        // Add submodels as children
        for (uint32_t i = 0; i < model->GetSubmodels().size(); i++)
            LoadModelNode(AssetManager::Request<Model>(model->GetSubmodels()[i]), modelEntity);
    }

    void DebutantLayer::DrawTransformGizmos()
    {
        // Draw gizmos
        Entity currSelection = m_SceneHierarchy.GetSelectionContext();

        if (currSelection && !m_PhysicsSelection.Dragging)
        {
            float winWidth = ImGui::GetWindowWidth();
            float winHeight = ImGui::GetWindowHeight();

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, winWidth, winHeight);

            auto& tc = currSelection.Transform();
            glm::mat4 transform = tc.GetTransform();

            const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();
            const glm::mat4& cameraProj = m_EditorCamera.GetProjection();

            bool snapping = Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL);
            float snapAmount = 0.5f;
            if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                snapAmount = 45;
            float snapValues[] = { snapAmount, snapAmount, snapAmount };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snapping ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 finalTrans, finalRot, finalScale;
                transform = (tc.Parent ? glm::inverse(tc.Parent.Transform().GetTransform()) : glm::mat4(1.0)) * transform;
                Math::DecomposeTransform(transform, finalTrans, finalRot, finalScale);

                glm::vec3 deltaRot = finalRot - tc.Rotation;

                tc.Translation = finalTrans;
                tc.Rotation += deltaRot;
                tc.Scale = finalScale;
            }
        }
    }

    void DebutantLayer::DrawPhysicsGizmos()
    {
        Entity currSelection = m_SceneHierarchy.GetSelectionContext();
        glm::vec4 viewport = glm::vec4(0.0f, 0.0f, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

        if (currSelection)
        {
            TransformComponent& transform = currSelection.Transform();
            // 2D PHYSICS GIZMOS
            // Get vertices and gizmos
            Collider2DComponent::Collider2DType colliderType = Collider2DComponent::Collider2DType::None;
            if (currSelection.HasComponent<BoxCollider2DComponent>())
                colliderType = Collider2DComponent::Collider2DType::Box;
            else if (currSelection.HasComponent<CircleCollider2DComponent>())
                colliderType = Collider2DComponent::Collider2DType::Circle;

            RendererDebug::BeginScene(m_EditorCamera, glm::inverse(m_EditorCamera.GetView()));

            switch (colliderType)
            {
            case Collider2DComponent::Collider2DType::Box:
            {
                BoxCollider2DComponent& boxCollider = currSelection.GetComponent<BoxCollider2DComponent>();
                RendererDebug::DrawRect(transform.GetTransform(), boxCollider.Size, boxCollider.Offset, { 0.0, 1.0, 0.0, 1.0 }, false);

                glm::mat4 transformMat = transform.GetTransform();
                glm::vec2 size = boxCollider.Size;
                glm::vec2 offset = boxCollider.Offset;

                glm::vec3 topLeft = transformMat * glm::vec4(-size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f, 1.0f);
                glm::vec3 bottomLeft = transformMat * glm::vec4(-size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f, 1.0f);
                glm::vec3 topRight = transformMat * glm::vec4(size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f, 1.0f);
                glm::vec3 bottomRight = transformMat * glm::vec4(size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f, 1.0f);

                RendererDebug::DrawPoint(topRight, (m_PhysicsSelection.SelectedName != "TopRight" ? glm::vec4(0, 1, 0, 1) : glm::vec4(0.2, 0.5, 1, 1)));
                RendererDebug::DrawPoint(bottomRight, (m_PhysicsSelection.SelectedName != "BottomRight" ? glm::vec4(0, 1, 0, 1) : glm::vec4(0.2, 0.5, 1, 1)));
                RendererDebug::DrawPoint(topLeft, (m_PhysicsSelection.SelectedName != "TopLeft" ? glm::vec4(0, 1, 0, 1) : glm::vec4(0.2, 0.5, 1, 1)));
                RendererDebug::DrawPoint(bottomLeft, (m_PhysicsSelection.SelectedName != "BottomLeft" ? glm::vec4(0, 1, 0, 1) : glm::vec4(0.2, 0.5, 1, 1)));
                
                break;
            }

            case Collider2DComponent::Collider2DType::Circle:
            {
                break;
            }
            case Collider2DComponent::Collider2DType::Polygon:
            {
                break;
            }
            default:
                break;
            }

            // Render points

            RendererDebug::EndScene();

            // IMPLEMENT LOGIC
            // Selection and dragging
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // Get hovered color
                glm::vec2 coords = GetFrameBufferCoords();
                glm::vec4 pixel = m_FrameBuffer->ReadPixel(0, coords.x, coords.y);

                if (!(pixel.r == 0.0 && pixel.g == 255.0 && pixel.b == 0.0 && pixel.a == 255))
                    return;

                switch (colliderType)
                {
                case Collider2DComponent::Collider2DType::Box:
                {
                    BoxCollider2DComponent& boxCollider = currSelection.GetComponent<BoxCollider2DComponent>();
                    glm::mat4 viewProj = m_EditorCamera.GetViewProjection();
                    RendererDebug::DrawRect(transform.GetTransform(), boxCollider.Size, boxCollider.Offset, { 0.0, 1.0, 0.0, 1.0 }, false);

                    glm::mat4 transformMat = transform.GetTransform();
                    glm::vec2 size = boxCollider.Size;
                    glm::vec2 offset = boxCollider.Offset;

                    glm::vec3 points[4] = {
                        glm::vec3(-size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f), glm::vec3(-size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f),
                        glm::vec3(size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f), glm::vec3(size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f)
                    };
                    glm::vec3 newPoints[4];
                    std::string pointNames[4] = { "TopLeft", "BottomLeft", "TopRight", "BottomRight" };

                    for (uint32_t i = 0; i < 4; i++)
                    {
                        glm::vec3 screenPoint = TransformationUtils::WorldToScreenPos(points[i], viewProj, transformMat, viewport);
                        if (glm::distance(screenPoint, glm::vec3(coords, screenPoint.z)) < 6.0f)
                        {
                            m_PhysicsSelection.ColliderType = colliderType;
                            m_PhysicsSelection.Dragging = true;
                            m_PhysicsSelection.SelectedName = pointNames[i];
                            m_PhysicsSelection.SelectedPoint = points[i];
                            m_PhysicsSelection.SelectedEntity = currSelection;

                            glm::mat4 pointTransform = glm::translate(transformMat, glm::vec3(transformMat * glm::vec4(points[i], 1.0)));

                            // Manipulate this point (could be isolated to be the same function for each point?)
                            ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetView()), 
                                glm::value_ptr(m_EditorCamera.GetProjection()), ImGuizmo::TRANSLATE,
                                ImGuizmo::MODE::LOCAL, glm::value_ptr(pointTransform));

                           /* if (ImGuizmo::IsUsing())
                            {
                                glm::vec3 finalTrans, finalRot, finalScale;
                                glm::mat4 newTransform = glm::inverse(transformMat) * pointTransform;
                                Math::DecomposeTransform(newTransform, finalTrans, finalRot, finalScale);

                                newPoints[i] = finalTrans;
                            }
                            else
                                newPoints[i] = points[i];

                            boxCollider.Size = { std::fabs(newPoints[0].x - newPoints[2].x),std::fabs(newPoints[0].y - newPoints[1].y) };*/
                            // What if recycling ImGuizmo? Probably the best approach to save me some headaches
                        }
                    }

                    // At the end of this loop, only one point will be changed. Edit the others accordingly

                    break;
                }

                case Collider2DComponent::Collider2DType::Circle:
                {
                    break;
                }
                case Collider2DComponent::Collider2DType::Polygon:
                {
                    break;
                }
                default:
                    break;
                }
            }
            else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                // Edit 
            }
            else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                // Reset selection
                m_PhysicsSelection.ColliderType = Collider2DComponent::Collider2DType::None;
                m_PhysicsSelection.Dragging = false;
                m_PhysicsSelection.SelectedName = "";
                m_PhysicsSelection.SelectedPoint = { 0,0,0 };
                m_PhysicsSelection.SelectedEntity = {};
            }

            /*
                - Transform all necessary points
                - Do stuff in screen space instead of world space. Basically transform the points instead of the point of 
                  the mouse position. Which kinda sucks and apparently Unity does it with only the near camera plane.
                  Ask in the cherno server maybe? I really don't want to implement depth buffers right now...
                - Screw it. Thought about it and this seems the best way.
            */

            // Convert from screen to world
            /*glm::vec3 worldSpaceCoords = glm::unProject(glm::vec3(coords.x, coords.y, 0.1f), m_EditorCamera.GetView(), m_EditorCamera.GetProjection(),
                glm::vec4(0, 0, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y));*/

        }
    }

    void DebutantLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        if (m_SceneState == SceneState::Edit)
            m_EditorCamera.OnEvent(e);

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
            m_ActiveScene->DuplicateEntity(m_SceneHierarchy.GetSelectionContext());
            break;
        // File Menu
        case DBT_KEY_N:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                NewScene();
            break;
        case DBT_KEY_O:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                OpenScene();
            break;
        case DBT_KEY_S:
            if (Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL) || Input::IsKeyPressed(DBT_KEY_RIGHT_CONTROL))
                if (Input::IsKeyPressed(DBT_KEY_LEFT_SHIFT) || Input::IsKeyPressed(DBT_KEY_RIGHT_SHIFT) || m_ScenePath == "")
                    SaveSceneAs();
                else
                    SaveScene();
            break;
        case DBT_KEY_1:
            m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case DBT_KEY_2:
            m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case DBT_KEY_3:
            m_GizmoType = ImGuizmo::OPERATION::SCALE;
            break;
        case DBT_KEY_4:
            m_GizmoType = ImGuizmo::OPERATION::UNIVERSAL;
            break;

        default:
            break;
        }

        return true;
    }

    bool DebutantLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        // Mouse picking
        if (e.GetMouseButton() == DBT_MOUSE_BUTTON_LEFT && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
        {
            glm::vec2 mouseCoords = GetFrameBufferCoords();
            int col = m_FrameBuffer->ReadRedPixel(1, mouseCoords.x, mouseCoords.y);

            if (col < 0)
                m_HoveredEntity = {};
            else
            {
                Entity entity = { (entt::entity)col, m_ActiveScene.get()};
                if (entity.IsValid())
                    m_HoveredEntity = entity;
                else
                    m_HoveredEntity = {};

                m_SceneHierarchy.SetSelectedEntity(m_HoveredEntity);
            }
        }

        return true;
    }

    glm::vec2 DebutantLayer::GetFrameBufferCoords()
    {
        auto [mouseX, mouseY] = ImGui::GetMousePos();
        mouseX -= m_ViewportBounds[0].x;
        mouseY -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

        int intMouseX = (int)mouseX;
        int intMouseY = (int)(viewportSize.y - mouseY);

        return { intMouseX, intMouseY };
    }

    void DebutantLayer::NewScene()
    {
        OnSceneStop();

        m_EditorScene = CreateRef<Scene>();
        m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);

        m_ActiveScene = m_EditorScene;
        m_RuntimeScene = nullptr;

        m_SceneHierarchy.SetContext(m_ActiveScene);
        m_SceneHierarchy.RebuildSceneGraph();
        m_ScenePath = "";
    }

    void DebutantLayer::OpenScene()
    {
        std::string path = FileDialogs::OpenFile("Debut Scene (*.debut)\0*.debut\0");
        if (!path.empty())
            OpenScene(path);
    }

    void DebutantLayer::OpenScene(std::filesystem::path path)
    {
        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        m_RuntimeScene = nullptr;
        m_EditorScene = CreateRef<Scene>();

        SceneSerializer ss(m_EditorScene);
        EntitySceneNode* sceneHierarchy = ss.DeserializeText(path.string());

        m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_ScenePath = path.string();
        m_ActiveScene = m_EditorScene;

        m_SceneHierarchy.SetContext(m_ActiveScene);
        m_SceneHierarchy.LoadTree(sceneHierarchy);
        m_SceneHierarchy.RebuildSceneGraph();
    }

    void DebutantLayer::SaveScene()
    {
        if (m_ScenePath == "")
        {
            SaveSceneAs();
            return;
        }
        SceneSerializer ss(m_ActiveScene);
        ss.SerializeText(m_ScenePath, *m_SceneHierarchy.GetSceneGraph());
    }

    void DebutantLayer::SaveSceneAs()
    {
        std::string path = FileDialogs::SaveFile("Debut Scene (*.debut)\0*.debut\0");
        if (!path.empty())
        {
            if (!CppUtils::String::EndsWith(path, ".debut"))
                path += ".debut";
            SceneSerializer ss(m_ActiveScene);
            ss.SerializeText(path, *m_SceneHierarchy.GetSceneGraph());

            m_ScenePath = path;
        }
    }
}
