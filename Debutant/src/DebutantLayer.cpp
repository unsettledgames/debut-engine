#include "DebutantLayer.h"
#include <Debut/dbtpch.h>
#include <Debut/Utils/PlatformUtils.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


namespace Debut
{
    void DebutantLayer::OnAttach()
    {
        FrameBufferSpecs fbSpecs;

        fbSpecs.Width = Application::Get().GetWindow().GetWidth();
        fbSpecs.Height = Application::Get().GetWindow().GetHeight();

        m_Texture = Texture2D::Create("C:/dev/Debut/Sandbox/assets/tileset.png");
        m_Checkerboard = Texture2D::Create("C:/dev/Debut/Debut/assets/textures/checkerboard.png");
        m_CameraController.SetZoomLevel(2);
        m_BushTexture = SubTexture2D::CreateFromCoords(m_Texture, glm::vec2(0, 4), glm::vec2(5, 6), glm::vec2(16, 16));
        m_FrameBuffer = FrameBuffer::Create(fbSpecs);

        m_ActiveScene = CreateRef<Scene>();
        
        m_SquareEntity = m_ActiveScene->CreateEntity("Square");
        m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 0.5f, 0.8f, 1.0f));

        auto square = m_ActiveScene->CreateEntity("Square 2");
        square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 0.8f, 0.5f, 1.0f));
        square.GetComponent<TransformComponent>().Translation = glm::vec3(1, 0, 0);


        class CameraController : public ScriptableEntity
        {
        public:
            void OnCreate()
            {
                Log.AppInfo("Camera controller created");
            }

            void OnUpdate(Timestep ts)
            {
                float cameraSpeed = 5;
                auto& transform = GetComponent<TransformComponent>();

                if (Input::IsKeyPressed(DBT_KEY_A))
                    transform.Translation.x -= ts * cameraSpeed;
                if (Input::IsKeyPressed(DBT_KEY_D))
                    transform.Translation.x += ts * cameraSpeed;
                if (Input::IsKeyPressed(DBT_KEY_W))
                    transform.Translation.y += ts * cameraSpeed;
                if (Input::IsKeyPressed(DBT_KEY_S))
                    transform.Translation.y -= ts * cameraSpeed;

            }
        };
        m_Camera = m_ActiveScene->CreateEntity("Camera");
        m_Camera.AddComponent<CameraComponent>();
        m_Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>();

        m_SceneHierarchy.SetContext(m_ActiveScene);
    }

    void DebutantLayer::OnDetach()
    {

    }

    void DebutantLayer::OnUpdate(Timestep ts)
    {
        // Update camera
        if (m_ViewportFocused)
            m_CameraController.OnUpdate(ts);

        Renderer2D::ResetStats();
        {
            DBT_PROFILE_SCOPE("Debutant::RendererSetup");
            m_FrameBuffer->Bind();

            RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
            RenderCommand::Clear();
        }

        // Update the scene
        m_ActiveScene->OnUpdate(ts);
        

        //Log.AppInfo("Frame time: {0}", (1.0f / ts));
        m_FrameBuffer->Unbind();
    }

    void DebutantLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        m_CameraController.OnEvent(e);

        dispatcher.Dispatch<KeyPressedEvent>(DBT_BIND(DebutantLayer::OnKeyPressed));
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

            ImGui::EndMenuBar();
        }

        m_SceneHierarchy.OnImGuiRender();

        ImGui::Begin("Settings");

            // Renderer2D stats
            ImGui::Text("Renderer2D Stats:");
            ImGui::Text("Draw calls: %d", stats.DrawCalls);
            ImGui::Text("Quads: %d", stats.QuadCount);
            ImGui::Text("Vertex count: %d", stats.GetTotalVertexCount());
            ImGui::Text("Index count: %d", stats.GetIndexCount());
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene view");
            
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused || !m_ViewportHovered);

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize.x != viewportSize.x || m_ViewportSize.y != viewportSize.y)
        {
            m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);

            m_FrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
            m_CameraController.Resize(m_ViewportSize.x, m_ViewportSize.y);

            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }
        uint32_t texId = m_FrameBuffer->GetColorAttachment();
        ImGui::Image((void*)texId, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2{ 0,1 }, ImVec2{ 1,0 });

        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::End();
    }

    bool DebutantLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        switch (e.GetKeyCode())
        {
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
        default:
            break;
        }

        return true;
    }

    void DebutantLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneHierarchy.SetContext(m_ActiveScene);

        m_ScenePath = "";
    }

    void DebutantLayer::OpenScene()
    {
        std::string path = FileDialogs::OpenFile("Debut Scene (*.debut)\0*.debut\0");
        if (!path.empty())
        {
            m_ActiveScene = CreateRef<Scene>();
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_SceneHierarchy.SetContext(m_ActiveScene);

            SceneSerializer ss(m_ActiveScene);
            ss.DeserializeText(path);

            m_ScenePath = path;
        }
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
