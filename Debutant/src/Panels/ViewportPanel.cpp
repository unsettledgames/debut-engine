#include <Panels/ViewportPanel.h>
#include <DebutantLayer.h>
#include <DebutantApp.h>
#include <Debut/Core/Application.h>
#include <Debut/Core/Instrumentor.h>
#include <Debut/Core/Window.h>
#include <Debut/ImGui/ImGuiLayer.h>
#include <Debut/ImGui/ImGuiUtils.h>

#include <Debut/Scene/Scene.h>
#include <Debut/Rendering/RenderTexture.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Structures/ShadowMap.h>
#include <Debut/Rendering/Renderer/Renderer.h>
#include <Debut/Rendering/Renderer/RendererDebug.h>

#include <Debut/Utils/TransformationUtils.h>

#include <filesystem>
#include <imgui.h>

/**
    USES OF DEBUTANTLAYER:
        - Loading droppable assets
*/

namespace Debut
{
    ViewportPanel::ViewportPanel(DebutantLayer* layer) : m_ParentLayer(layer)
    {
        FrameBufferSpecifications sceneFbSpecs;
        FrameBufferSpecifications textureFbSpecs;

        sceneFbSpecs.Attachments = {
            FrameBufferTextureFormat::Color, FrameBufferTextureFormat::Depth,
            FrameBufferTextureFormat::RED_INTEGER
        };

        textureFbSpecs.Attachments = { FrameBufferTextureFormat::Color };

        sceneFbSpecs.Width = DebutantApp::Get().GetWindow().GetWidth();
        sceneFbSpecs.Height = DebutantApp::Get().GetWindow().GetHeight();

        textureFbSpecs.Width = sceneFbSpecs.Width;
        textureFbSpecs.Height = sceneFbSpecs.Height;

        m_SceneFrameBuffer = FrameBuffer::Create(sceneFbSpecs);
        m_TextureFrameBuffer = FrameBuffer::Create(textureFbSpecs);

        m_RenderTexture = RenderTexture::Create(sceneFbSpecs.Width, sceneFbSpecs.Height, 
            DebutantApp::Get().GetSceneManager().GetActiveScene()->GetShadowMaps()[0]->GetFrameBuffer(), RenderTextureMode::Color);
        m_FullscreenShader = AssetManager::Request<Shader>("assets\\shaders\\fullscreenquad.glsl");

        m_EditorCamera = EditorCamera(30, 16.0f / 9.0f, 0.1f, 1000.0f);
    }

    void ViewportPanel::OnUpdate(Timestep& ts)
    {
        DBT_PROFILE_SCOPE("EgineUpdate");
        //Log.CoreInfo("FPS: {0}", 1.0f / ts);
        // Update camera
        if (m_ViewportFocused)
            m_EditorCamera.OnUpdate(ts);

        SceneManager::SceneState sceneState = DebutantApp::Get().GetSceneManager().GetState();
        Ref<Scene> activeScene = DebutantApp::Get().GetSceneManager().GetActiveScene();
        // Update the scene
        switch (sceneState)
        {
        case SceneManager::SceneState::Play:
            activeScene->OnRuntimeUpdate(ts, m_SceneFrameBuffer);
            break;
        case SceneManager::SceneState::Edit:
            activeScene->OnEditorUpdate(ts, m_EditorCamera, m_SceneFrameBuffer);

            m_SceneFrameBuffer->Bind();
            // Render debug
            DrawCollider();
            break;
        }
        m_SceneFrameBuffer->Unbind();

        // The scene frame buffer now contains the whole scene. Render the frame buffer to a texture.
        m_TextureFrameBuffer->Bind();
        m_RenderTexture->SetFrameBuffer(m_SceneFrameBuffer);
        m_RenderTexture->Draw(m_FullscreenShader);
        m_TextureFrameBuffer->Unbind();
    }

	void ViewportPanel::OnImGuiRender()
	{
        ImGui::Begin("ShadowMap");
        {
            static int shadowMapIndex = 0;

            Ref<Scene> activeScene = DebutantApp::Get().GetSceneManager().GetActiveScene();
            uint32_t rendererID = activeScene->GetShadowMaps()[shadowMapIndex]->GetFrameBuffer()->GetDepthAttachment();

            ImGui::Image((void*)rendererID, { 300, 300 }, { 0, 1 }, { 1, 0 });
            ImGui::DragInt("Shadowmap index", &shadowMapIndex);
            ImGui::DragFloat("Camera near", &activeScene->cameraNear);
            ImGui::DragFloat("Camera far", &activeScene->cameraFar);
            ImGui::DragFloat("Ortho size", &activeScene->orthoSize);
            ImGui::DragFloat("Camera distance", &activeScene->cameraDistance);
            ImGui::DragFloat("Fadeout start distance", &activeScene->fadeoutStartDistance);
            ImGui::DragFloat("Fadeout end distance", &activeScene->fadeoutEndDistance);

            shadowMapIndex = std::min(std::max(0, shadowMapIndex), 3);
        }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, ImGui::GetTextLineHeight() });
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
        {
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            ImVec2 menuSize;

            // Overlay menu bar
            if (ImGui::BeginMenuBar())
            {
                ImVec2 desiredSize = { viewportSize.x, ImGui::GetTextLineHeight() * 1.5f };
                ImGui::PopStyleVar();

                DrawTopBar(desiredSize);
            }
            ImGui::EndMenuBar();

            // Compute menu size
            m_TopMenuSize = { ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y };
            m_TopMenuSize.y += ImGui::GetTextLineHeight() * 1.5f;

            // Draw scene
            uint32_t texId = m_TextureFrameBuffer->GetColorAttachment();
            ImGui::Image((void*)texId, ImVec2(viewportSize.x, viewportSize.y), ImVec2{ 0,1 }, ImVec2{ 1,0 });

            // Accept scene loading
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA");
                if (payload != nullptr)
                {
                    std::filesystem::path path((const wchar_t*)payload->Data);
                    if (path.extension() == ".debut")
                        m_ParentLayer->OnOpenScene(path);
                    else if (path.extension() == ".model")
                        m_ParentLayer->LoadModel(path);
                    ImGui::EndDragDropTarget();
                }
            }

            // Window resizing
            auto viewportOffset = ImGui::GetCursorPos();

            m_ViewportFocused = ImGui::IsWindowFocused();
            m_ViewportHovered = ImGui::IsWindowHovered();
            Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

            if (m_ViewportSize.x != viewportSize.x || m_ViewportSize.y != viewportSize.y)
            {
                m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);

                m_SceneFrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
                m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
                DebutantApp::Get().GetSceneManager().GetActiveScene()->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            }

            // Save bounds for mouse picking
            ImVec2 minBound = ImGui::GetItemRectMin();
            ImVec2 maxBound = ImGui::GetItemRectMax();
            SceneManager::SceneState sceneState = DebutantApp::Get().GetSceneManager().GetState();
            m_ViewportBounds[0] = { minBound.x, minBound.y };
            m_ViewportBounds[1] = { maxBound.x, maxBound.y - menuSize.y };

            if (sceneState == SceneManager::SceneState::Edit)
            {
                ImVec2 windowPos = ImGui::GetWindowPos();
                windowPos.y += m_TopMenuSize.y;

                if (m_Selection && m_Selection.IsValid())
                {
                    if (!m_ColliderSelection.Valid)
                        m_Gizmos.ManipulateTransform(m_Selection, m_EditorCamera, viewportSize, windowPos);
                    else
                        m_Gizmos.ManipulateCollider(m_Selection, m_EditorCamera, viewportSize, windowPos, m_ColliderSelection);
                }
            }

            ImGui::PopStyleVar();
        }
        ImGui::End();
	}

	void ViewportPanel::DrawTopBar(ImVec2& menuSize)
	{
        float buttonSize = ImGui::GetTextLineHeight() * 2.0f;
        float bigButtonSize = buttonSize * 1.5f;
        float verticalCenter = (menuSize.y - buttonSize) + buttonSize * 0.5f;
        float bigVerticalCenter = (menuSize.y - bigButtonSize) + bigButtonSize * 0.5f;

        RendererConfig currSceneConfig = Renderer::GetConfig();

        ImGui::SetCursorPos({ (menuSize.x - buttonSize * 1.5f) * 0.5f, bigVerticalCenter });
        SceneManager::SceneState sceneState = DebutantApp::Get().GetSceneManager().GetState();
        // Play icon
        if (ImGui::Button(sceneState == SceneManager::SceneState::Edit ? IMGUI_ICON_PLAY : IMGUI_ICON_STOP,
            ImVec2(buttonSize * 1.5f, buttonSize * 1.5f)))
        {
            SceneManager::SceneState sceneState = DebutantApp::Get().GetSceneManager().GetState();
            // TODO: pause scene
            if (sceneState == SceneManager::SceneState::Edit)
                DebutantApp::Get().GetSceneManager().OnScenePlay();
            else if (sceneState == SceneManager::SceneState::Play)
                DebutantApp::Get().GetSceneManager().OnSceneStop();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3, 3 });
        ImGui::SetCursorPos({ 10, verticalCenter });

        // Gizmo mode
        Gizmos::GizmoMode currMode = m_Gizmos.GetMode();
        if (ImGui::Button(currMode == Gizmos::GizmoMode::Local ? IMGUI_ICON_GIZMO_GLOBAL : IMGUI_ICON_GIZMO_LOCAL, {buttonSize, buttonSize}))
            m_Gizmos.SetMode(currMode == Gizmos::GizmoMode::Local ? Gizmos::GizmoMode::Global : Gizmos::GizmoMode::Local);

        // Translation rotation scale buttons
        Gizmos::GizmoOperation operations[3] = { Gizmos::GizmoOperation::Translate, 
            Gizmos::GizmoOperation::Rotate, Gizmos::GizmoOperation::Scale };
        const char* icons[3] = { IMGUI_ICON_TRANSLATE, IMGUI_ICON_ROTATE, IMGUI_ICON_SCALE };

        for (uint32_t i = 0; i < 3; i++)
        {
            ImGui::SetCursorPosY(verticalCenter);
            if (m_Gizmos.GetOperation() == operations[i])
            {
                ScopedStyleColor col(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
                if (ImGui::Button(icons[i], { buttonSize, buttonSize }))
                    m_Gizmos.SetOperation(operations[i]);
            }
            else if (ImGui::Button(icons[i], { buttonSize, buttonSize }))
                m_Gizmos.SetOperation(operations[i]);
        }

        // Rendering modes
        ImVec2 padding(5.0f, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, padding);

        ImGui::SetCursorPos({ menuSize.x - 190.0f - padding.x, -padding.y + verticalCenter + ImGui::GetTextLineHeight() / 2.0f });
        ImGui::SetNextItemWidth(170.0f);

        if (ImGui::BeginCombo("##renderingmode", "Rendering options", ImGuiComboFlags_HeightLargest))
        {
            if (ImGui::Selectable("Standard", currSceneConfig.RenderingMode == RendererConfig::RenderingMode::Standard))
                currSceneConfig.RenderingMode = RendererConfig::RenderingMode::Standard;
            if (ImGui::Selectable("Untextured", currSceneConfig.RenderingMode == RendererConfig::RenderingMode::Untextured))
                currSceneConfig.RenderingMode = RendererConfig::RenderingMode::Untextured;
            if (ImGui::Selectable("Depth buffer", currSceneConfig.RenderingMode == RendererConfig::RenderingMode::Depth))
                currSceneConfig.RenderingMode = RendererConfig::RenderingMode::Depth;
            if (ImGui::Selectable("None", currSceneConfig.RenderingMode == RendererConfig::RenderingMode::None))
                currSceneConfig.RenderingMode = RendererConfig::RenderingMode::None;

            ImGuiUtils::Separator();
            {
                ScopedStyleVar var(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
                ImGui::Checkbox("Render wireframe", &currSceneConfig.RenderWireframe);
            }
            {
                ScopedStyleVar var(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
                ImGui::Checkbox("Render colliders", &currSceneConfig.RenderColliders);
            }

            ImGui::Dummy({ 0.0f, 3.0f });

            ImGui::EndCombo();
        }

        Renderer::SetConfig(currSceneConfig);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
	}

    void ViewportPanel::DrawCollider()
    {
        DBT_PROFILE_FUNCTION();
        glm::vec2* viewportBounds = GetViewportBounds();

        Entity currSelection = m_Selection;
        glm::vec4 viewport = glm::vec4(0.0f, 0.0f, viewportBounds[1].x - viewportBounds[0].x, viewportBounds[1].y - viewportBounds[0].y);
        // Points
        std::vector<glm::vec3> points;
        // Labels
        std::vector<std::string> labels;

        if (currSelection)
        {
            TransformComponent& transform = currSelection.Transform();
            glm::mat4 transformMat = transform.GetTransform();
            glm::mat4 viewProj = m_EditorCamera.GetViewProjection();

            RendererDebug::BeginScene(m_EditorCamera, glm::inverse(m_EditorCamera.GetView()));

            if (currSelection.HasComponent<BoxCollider2DComponent>())
            {
                DBT_PROFILE_SCOPE("Debutant::DrawPhysicsGizmos::BoxCollider2D");

                BoxCollider2DComponent& boxCollider = currSelection.GetComponent<BoxCollider2DComponent>();
                glm::vec2 size = boxCollider.Size;
                glm::vec2 offset = boxCollider.Offset;

                points = {
                        glm::vec3(-size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f), glm::vec3(-size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f),
                        glm::vec3(size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f), glm::vec3(size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f)
                };
                labels = { "TopLeft", "BottomLeft", "TopRight", "BottomRight" };

                glm::vec3 topLeft = transformMat * glm::vec4(points[0], 1.0f);
                glm::vec3 bottomLeft = transformMat * glm::vec4(points[1], 1.0f);
                glm::vec3 topRight = transformMat * glm::vec4(points[2], 1.0f);
                glm::vec3 bottomRight = transformMat * glm::vec4(points[3], 1.0f);

                RendererDebug::DrawRect(transformMat, boxCollider.Size, boxCollider.Offset, { 0.0, 1.0, 0.0, 1.0 }, false);

                for (uint32_t i = 0; i < 4; i++)
                    RendererDebug::DrawPoint(transformMat * glm::vec4(points[i], 1.0f), glm::vec4(0, 1, 0, 1));
                m_ColliderSelection.PointTransform = transformMat;
            }
            else if (currSelection.HasComponent<CircleCollider2DComponent>())
            {
                DBT_PROFILE_SCOPE("Debutant::DrawPhysicsGizmos::CircleCollider2D");

                CircleCollider2DComponent& cc = currSelection.GetComponent<CircleCollider2DComponent>();
                glm::vec3 center = glm::vec3(cc.Offset, 0.0f);
                points = {
                        glm::vec3(-cc.Radius + cc.Offset.x, cc.Offset.y, 0.0f), glm::vec3(cc.Offset.x, cc.Radius + cc.Offset.y, 0.0f),
                        glm::vec3(cc.Radius + cc.Offset.x, cc.Offset.y, 0.0f), glm::vec3(cc.Offset.x, -cc.Radius + cc.Offset.y, 0.0f)
                };
                labels = { "Left", "Top", "Right", "Bottom" };

                RendererDebug::DrawCircle(cc.Radius, center, transform.GetTransform(), 40);
                for (uint32_t i = 0; i < 4; i++)
                    RendererDebug::DrawPoint(glm::vec3(transformMat * glm::vec4(points[i], 1.0f)) / transform.Scale, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                m_ColliderSelection.PointTransform = transformMat;
            }
            else if (currSelection.HasComponent<PolygonCollider2DComponent>())
            {
                DBT_PROFILE_SCOPE("Debutant::DrawPhysicsGizmos::PolygonCollider2D");

                PolygonCollider2DComponent& polygon = currSelection.GetComponent<PolygonCollider2DComponent>();
                glm::vec3 center = glm::vec3(polygon.Offset, 0.0f);

                // Render points
                for (uint32_t i = 0; i < polygon.Points.size(); i++)
                {
                    glm::vec2 currPoint = polygon.Points[i];
                    std::stringstream ss;
                    ss << i;
                    points.push_back(glm::vec3(currPoint, 0.0f));
                    labels.push_back(ss.str());

                    RendererDebug::DrawPoint(glm::vec3(transformMat * glm::vec4(center + glm::vec3(currPoint, 0.0f), 1.0f)), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                }
                RendererDebug::DrawPolygon(polygon.GetTriangles(), glm::vec3(polygon.Offset, transform.Translation.z),
                    transformMat, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                m_ColliderSelection.PointTransform = transformMat;
            }
            else if (currSelection.HasComponent<BoxCollider3DComponent>())
            {
                DBT_PROFILE_SCOPE("Debutant::DrawPhysicsGizmos::BoxCollider3D");

                BoxCollider3DComponent& collider = currSelection.GetComponent<BoxCollider3DComponent>();
                glm::vec3 center = collider.Offset;
                glm::vec3 hSize = collider.Size / 2.0f;

                // Fill points
                points = {
                    glm::vec3(hSize.x, hSize.y, hSize.z) + center,
                    glm::vec3(-hSize.x, hSize.y, hSize.z) + center,
                    glm::vec3(hSize.x,-hSize.y, hSize.z) + center,
                    glm::vec3(-hSize.x,-hSize.y, hSize.z) + center,
                    glm::vec3(hSize.x, hSize.y,-hSize.z) + center,
                    glm::vec3(-hSize.x, hSize.y,-hSize.z) + center,
                    glm::vec3(hSize.x,-hSize.y,-hSize.z) + center,
                    glm::vec3(-hSize.x,-hSize.y,-hSize.z) + center
                };

                labels = { "0", "1", "2", "3", "4", "5", "6", "7" };
                // Fill labels
                for (uint32_t i = 0; i < points.size(); i++)
                {
                    std::stringstream ss;
                    ss << i;
                    labels.push_back(ss.str());
                }

                RendererDebug::DrawBox(collider.Size, collider.Offset, transformMat, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                m_ColliderSelection.PointTransform = transformMat;
            }
            else if (currSelection.HasComponent<SphereCollider3DComponent>())
            {
                DBT_PROFILE_SCOPE("Debutant::DrawPhysicsGizmos::SphereCollider3D");

                glm::vec3 trans, rot, scale;
                MathUtils::DecomposeTransform(transform.GetTransform(), trans, rot, scale);
                SphereCollider3DComponent& collider = currSelection.GetComponent<SphereCollider3DComponent>();
                float radius = collider.Radius;
                glm::mat4 circleTransform = MathUtils::CreateTransform(trans, rot, glm::vec3(glm::compMin(scale)));

                // Use it to draw
                RendererDebug::DrawSphere(radius, collider.Offset, rot, scale, glm::mat4(glm::mat3(m_EditorCamera.GetView())),
                    circleTransform);

                points = {
                    glm::vec3(radius, 0.0f, 0.0f) + collider.Offset, glm::vec3(-radius, 0.0f, 0.0f) + collider.Offset,
                    glm::vec3(0.0f, radius, 0.0f) + collider.Offset, glm::vec3(0.0f, -radius, 0.0f) + collider.Offset,
                    glm::vec3(0.0f, 0.0f, radius) + collider.Offset, glm::vec3(0.0f, 0.0f, -radius) + collider.Offset
                };
                labels = { "Right", "Left", "Top", "Down", "Front", "Bottom" };

                for (uint32_t i = 0; i < points.size(); i++)
                    RendererDebug::DrawPoint(circleTransform * glm::vec4(points[i], 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                m_ColliderSelection.PointTransform = circleTransform;
            }
            else if (currSelection.HasComponent<MeshCollider3DComponent>())
            {
                DBT_PROFILE_SCOPE("Debutant::DrawPhysicsGizmos::MeshCollider3D");

                MeshCollider3DComponent& collider = currSelection.GetComponent<MeshCollider3DComponent>();
                RendererDebug::DrawMesh(collider.Mesh, collider.Offset, transformMat, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            }

            // Render points
            RendererDebug::EndScene();

            // IMPLEMENT LOGIC
            // Selection and dragging
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                // Don't edit the selection if the user is dragging a vertex
                if (ImGuizmo::IsUsing())
                    return;

                // Get hovered color
                glm::vec2 coords = GetFrameBufferCoords();
                glm::vec4 pixel = m_SceneFrameBuffer->ReadPixel(0, coords.x, coords.y);

                // Check that the distance is far from the selected point before disabling TODO
                if (!(pixel.r == 0.0 && pixel.g == 255.0 && pixel.b == 0.0 && pixel.a == 255) &&
                    glm::distance(TransformationUtils::WorldToScreenPos(m_ColliderSelection.SelectedPoint,
                        m_EditorCamera.GetViewProjection(), m_ColliderSelection.PointTransform, viewport),
                        glm::vec3(coords, m_ColliderSelection.SelectedPoint.z)) > 6.0f)
                {
                    m_ColliderSelection.Valid = false;
                    m_ColliderSelection.SelectedName = "";
                    return;
                }

                for (uint32_t i = 0; i < points.size(); i++)
                {
                    glm::vec3 screenPoint = TransformationUtils::WorldToScreenPos(points[i], viewProj, m_ColliderSelection.PointTransform, viewport);
                    if (glm::distance(screenPoint, glm::vec3(coords, screenPoint.z)) < 6.0f)
                    {
                        m_ColliderSelection.Valid = true;
                        m_ColliderSelection.SelectedName = labels[i];
                        m_ColliderSelection.SelectedPoint = points[i];
                        m_ColliderSelection.SelectedEntity = currSelection;
                    }
                }
            }
        }
    }

    bool ViewportPanel::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        switch (e.GetKeyCode())
        {
        case DBT_KEY_1:
            m_Gizmos.SetOperation(Gizmos::GizmoOperation::Translate);
            break;
        case DBT_KEY_2:
            m_Gizmos.SetOperation(Gizmos::GizmoOperation::Rotate);
            break;
        case DBT_KEY_3:
            m_Gizmos.SetOperation(Gizmos::GizmoOperation::Scale);
            break;
        default:
            break;
        }

        return true;
    }

    glm::vec2 ViewportPanel::GetFrameBufferCoords()
    {
        auto [mouseX, mouseY] = ImGui::GetMousePos();
        glm::vec2* viewportBounds = GetViewportBounds();

        mouseX -= viewportBounds[0].x;
        mouseY -= viewportBounds[0].y;
        glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];

        int intMouseX = (int)mouseX;
        int intMouseY = (int)(viewportSize.y - mouseY);

        return { intMouseX, intMouseY };
    }
}