#include "DebutantLayer.h"
#include "Debut/Core/Instrumentor.h"
#include "Camera/EditorCamera.h"
#include <Utils/EditorCache.h>
#include <Debut/Rendering/Resources/Model.h>
#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/Utils/PlatformUtils.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>

#include <chrono>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

namespace Debutant
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
        m_ContentBrowser.SetPropertiesPanel(&m_PropertiesPanel);

        m_IconPlay = Texture2D::Create("assets\\icons\\play.png");
        m_IconStop = Texture2D::Create("assets\\icons\\stop.png");

        AssetManager::Init(".");

        EditorCache::Textures().Put("assets\\icons\\play.png", m_IconPlay);
        EditorCache::Textures().Put("assets\\icons\\stop.png", m_IconStop);

        AssetManager::Request<Shader>("assets\\shaders\\default-3d.glsl");
        
        /*ModelImporter::ImportModel("assets\\models\\house\\source\\domik2\\domik2.obj");
        m_Model = AssetManager::Request<Model>(
            AssetManager::Request<Model>("assets\\models\\house\\source\\domik2\\domik2.obj.model")->GetSubmodels()[0]);*/

        /*ModelImporter::ImportModel("assets\\models\\cube\\untitled.obj");
        m_Model = AssetManager::Request<Model>(
            AssetManager::Request<Model>("assets\\models\\house\\source\\domik2\\domik2.obj.model")->GetSubmodels()[0]);*/

        ModelImporter::ImportModel("assets\\models\\pyramid\\source\\1.obj");
        m_Model = AssetManager::Request<Model>(AssetManager::Request<Model>("assets\\models\\pyramid\\source\\1.obj.model")->GetSubmodels()[0]);

            /*
        ModelImporter::ImportModel("assets\\models\\car\\car.obj");
        m_Model = AssetManager::Request<Model>(
            AssetManager::Request<Model>("assets\\models\\car\\car.obj.model")->GetSubmodels()[0]);*/
    }

    void DebutantLayer::OnDetach()
    {

    }

    void DebutantLayer::OnUpdate(Timestep ts)
    {
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
            break;
        }

        Renderer3D::BeginScene(m_EditorCamera, glm::inverse(m_EditorCamera.GetView()));
        MeshRendererComponent component;

        component.Mesh = m_Model->GetMeshes()[0];
        component.Material = m_Model->GetMaterials()[0];

        Renderer3D::DrawModel(component, glm::mat4(1.0));
        Renderer3D::EndScene();

        m_FrameBuffer->Unbind();
    }

    void DebutantLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Play;

        // TODO: textures aren't updated in the runtime scene
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

            DrawViewport();
            DrawUIToolbar();

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
            // TODO: simulate physics, pause scene...
            if (m_SceneState == SceneState::Edit)
                OnScenePlay();
            else if (m_SceneState == SceneState::Play)
                OnSceneStop();
        }

        ImGui::End();
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

                if (ImGui::MenuItem("Exit")) Application::Get().Close();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets"))
            {
                if (ImGui::MenuItem("Reimport"))
                    AssetManager::Reimport();
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
                    OpenScene(std::filesystem::path((const wchar_t*)payload->Data));
                    ImGui::EndDragDropTarget();
                }

            }

            // Save bounds for mouse picking
            ImVec2 minBound = ImGui::GetItemRectMin();
            ImVec2 maxBound = ImGui::GetItemRectMax();
            m_ViewportBounds[0] = { minBound.x, minBound.y };
            m_ViewportBounds[1] = { maxBound.x, maxBound.y };

            if (m_SceneState == SceneState::Edit)
                DrawGizmos();

            ImGui::PopStyleVar();
        }
        ImGui::End();
    }

    void DebutantLayer::DrawGizmos()
    {
        // Draw gizmos
        Entity currSelection = m_SceneHierarchy.GetSelectionContext();

        bool snapping = Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL);
        float snapAmount = 0.5f;
        if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
            snapAmount = 45;
        float snapValues[] = { snapAmount, snapAmount, snapAmount };

        if (currSelection)
        {
            float winWidth = ImGui::GetWindowWidth();
            float winHeight = ImGui::GetWindowHeight();

            const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();
            const glm::mat4& cameraProj = m_EditorCamera.GetProjection();

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, winWidth, winHeight);

            auto& tc = currSelection.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snapping ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 finalTrans, finalRot, finalScale;
                Math::DecomposeTransform(transform, finalTrans, finalRot, finalScale);

                glm::vec3 deltaRot = finalRot - tc.Rotation;

                tc.Translation = finalTrans;
                tc.Rotation += deltaRot;
                tc.Scale = finalScale;
            }
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
        auto [mouseX, mouseY] = ImGui::GetMousePos();
        mouseX -= m_ViewportBounds[0].x;
        mouseY -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

        int intMouseX = (int)mouseX;
        int intMouseY = (int)(viewportSize.y - mouseY);

        if (e.GetMouseButton() == DBT_MOUSE_BUTTON_LEFT && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
        {
            if (m_ViewportHovered)
            {
                int hoveredID = m_FrameBuffer->ReadPixel(1, intMouseX, intMouseY);

                if (hoveredID < 0)
                    m_HoveredEntity = {};
                else
                    m_HoveredEntity = { (entt::entity)hoveredID, m_ActiveScene.get() };

                m_SceneHierarchy.SetSelectedEntity(m_HoveredEntity);
            }
        }

        return true;
    }

    void DebutantLayer::NewScene()
    {
        OnSceneStop();

        m_EditorScene = CreateRef<Scene>();
        m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);

        m_ActiveScene = m_EditorScene;
        m_RuntimeScene = nullptr;

        m_SceneHierarchy.SetContext(m_ActiveScene);
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

        m_EditorScene = CreateRef<Scene>();
        m_RuntimeScene = nullptr;

        m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneHierarchy.SetContext(m_EditorScene);

        SceneSerializer ss(m_EditorScene);
        ss.DeserializeText(path.string());

        m_ScenePath = path.string();
        m_ActiveScene = m_EditorScene;
    }

    void DebutantLayer::SaveScene()
    {
        if (m_ScenePath == "")
        {
            SaveSceneAs();
            return;
        }
        SceneSerializer ss(m_ActiveScene);
        ss.SerializeText(m_ScenePath);
    }

    void DebutantLayer::SaveSceneAs()
    {
        std::string path = FileDialogs::SaveFile("Debut Scene (*.debut)\0*.debut\0");
        if (!path.empty())
        {
            SceneSerializer ss(m_ActiveScene);
            ss.SerializeText(path);

            m_ScenePath = path;
        }
    }
}
