#include <Panels/ViewportPanel.h>
#include <DebutantLayer.h>

#include <Debut/Core/Application.h>
#include <Debut/ImGui/ImGuiLayer.h>
#include <Debut/ImGui/ImGuiUtils.h>

#include <Debut/Scene/Scene.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Renderer/Renderer.h>

#include <filesystem>
#include <imgui.h>

namespace Debut
{
	void ViewportPanel::OnImGuiRender()
	{
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
            uint32_t texId = m_FrameBuffer->GetColorAttachment();
            ImGui::Image((void*)texId, ImVec2(viewportSize.x, viewportSize.y), ImVec2{ 0,1 }, ImVec2{ 1,0 });

            // Accept scene loading
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA");
                if (payload != nullptr)
                {
                    std::filesystem::path path((const wchar_t*)payload->Data);
                    if (path.extension() == ".debut")
                        m_ParentLayer->OpenScene(path);
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

                m_FrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
                m_ParentLayer->m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
                m_Context->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            }

            // Save bounds for mouse picking
            ImVec2 minBound = ImGui::GetItemRectMin();
            ImVec2 maxBound = ImGui::GetItemRectMax();
            m_ViewportBounds[0] = { minBound.x, minBound.y };
            m_ViewportBounds[1] = { maxBound.x, maxBound.y - menuSize.y };

            if (m_ParentLayer->m_SceneState == DebutantLayer::SceneState::Edit)
                m_ParentLayer->DrawTransformGizmos();

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
        // Play icon
        if (ImGui::Button(m_ParentLayer->m_SceneState == DebutantLayer::SceneState::Edit ? IMGUI_ICON_PLAY : IMGUI_ICON_STOP,
            ImVec2(buttonSize * 1.5f, buttonSize * 1.5f)))
        {
            // TODO: pause scene
            if (m_ParentLayer->m_SceneState == DebutantLayer::SceneState::Edit)
                m_ParentLayer->OnScenePlay();
            else if (m_ParentLayer->m_SceneState == DebutantLayer::SceneState::Play)
                m_ParentLayer->OnSceneStop();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3, 3 });
        ImGui::SetCursorPos({ 10, verticalCenter });

        // Gizmo mode
        if (ImGui::Button(m_ParentLayer->m_GizmoMode == ImGuizmo::LOCAL ? IMGUI_ICON_GIZMO_GLOBAL : IMGUI_ICON_GIZMO_LOCAL, { buttonSize, buttonSize }))
            m_ParentLayer->m_GizmoMode = m_ParentLayer->m_GizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

        // Translation rotation scale buttons
        ImGuizmo::OPERATION operations[3] = { ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE };
        const char* icons[3] = { IMGUI_ICON_TRANSLATE, IMGUI_ICON_ROTATE, IMGUI_ICON_SCALE };

        for (uint32_t i = 0; i < 3; i++)
        {
            ImGui::SetCursorPosY(verticalCenter);
            if (m_ParentLayer->m_GizmoType == operations[i])
            {
                ScopedStyleColor col(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
                if (ImGui::Button(icons[i], { buttonSize, buttonSize }))
                    m_ParentLayer->m_GizmoType = operations[i];
            }
            else if (ImGui::Button(icons[i], { buttonSize, buttonSize }))
                m_ParentLayer->m_GizmoType = operations[i];
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
}