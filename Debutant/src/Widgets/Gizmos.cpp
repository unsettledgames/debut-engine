#include <Widgets/Gizmos.h>
#include <Camera/EditorCamera.h>

#include <Debut/Core/Input.h>
#include <Debut/Core/KeyCodes.h>
#include <Debut/Core/Instrumentor.h>

#include <Debut/Scene/Entity.h>
#include <Debut/Scene/Components.h>

#include <Debut/Rendering/Renderer/RendererDebug.h>

#include <Debut/Utils/TransformationUtils.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include <vector>
#include <sstream>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Debut
{
    static ImGuizmo::OPERATION DbtToImguizmoOperation(const Gizmos::GizmoOperation& op)
    {
        switch (op)
        {
        case Gizmos::GizmoOperation::Translate:
            return ImGuizmo::TRANSLATE;
        case Gizmos::GizmoOperation::Rotate:
            return ImGuizmo::ROTATE;
        case Gizmos::GizmoOperation::Scale:
            return ImGuizmo::SCALE;
        default:
            return ImGuizmo::TRANSLATE;
        }
    }
    static ImGuizmo::MODE DbtToImguizmoMode(const Gizmos::GizmoMode& mode)
    {
        switch (mode)
        {
        case Gizmos::GizmoMode::Local:
            return ImGuizmo::LOCAL;
        case Gizmos::GizmoMode::Global:
            return ImGuizmo::WORLD;
        default:
            return ImGuizmo::WORLD;
        }
    }

	void Gizmos::ManipulateTransform(Entity& currSelection, const EditorCamera& camera, ImVec2& viewportSize, ImVec2& windowPos)
	{
        DBT_PROFILE_FUNCTION();

        if (currSelection)
        {
            float winWidth = ImGui::GetWindowWidth();
            float winHeight = ImGui::GetWindowHeight();

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(windowPos.x, windowPos.y, viewportSize.x, viewportSize.y);

            auto& tc = currSelection.Transform();
            glm::mat4 transform = tc.GetTransform();

            const glm::mat4& cameraView = camera.GetView();
            const glm::mat4& cameraProj = camera.GetProjection();

            bool snapping = Input::IsKeyPressed(DBT_KEY_LEFT_CONTROL);
            float snapAmount = 0.5f;
            if (m_GizmoOperation == GizmoOperation::Rotate)
                snapAmount = 45;
            float snapValues[] = { snapAmount, snapAmount, snapAmount };

            if (ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                DbtToImguizmoOperation(m_GizmoOperation), DbtToImguizmoMode(m_GizmoMode), 
                glm::value_ptr(transform), nullptr, snapping ? snapValues : nullptr))
            {
                glm::vec3 finalTrans, finalRot, finalScale;
                transform = (tc.Parent ? glm::inverse(tc.Parent.Transform().GetTransform()) : glm::mat4(1.0)) * transform;
                MathUtils::DecomposeTransform(transform, finalTrans, finalRot, finalScale);

                glm::vec3 deltaRot = finalRot - tc.Rotation;

                tc.Translation = finalTrans;
                tc.Rotation += deltaRot;
                tc.Scale = finalScale;
            }
        }
	}

    void Gizmos::ManipulateCollider(Entity& currSelection, const EditorCamera& camera, ImVec2& viewportSize, 
        ImVec2& windowPos, PhysicsColliderSelection& point)
    {
        DBT_PROFILE_FUNCTION();

        if (currSelection && point.Valid)
        {
            float winWidth = ImGui::GetWindowWidth();
            float winHeight = ImGui::GetWindowHeight();

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(windowPos.x, windowPos.y, viewportSize.x, viewportSize.y);

            // Convert from collider space to world space
            glm::mat4 pointTransform = glm::translate(point.PointTransform, point.SelectedPoint);

            // Manipulate the selected point
            if (ImGuizmo::Manipulate(glm::value_ptr(camera.GetView()),
                glm::value_ptr(camera.GetProjection()), ImGuizmo::TRANSLATE,
                DbtToImguizmoMode(m_GizmoMode), glm::value_ptr(pointTransform)))
            {
                // Apply the changes
                glm::vec3 newPoint = point.SelectedPoint;

                glm::vec3 finalTrans, finalRot, finalScale;
                MathUtils::DecomposeTransform(pointTransform, finalTrans, finalRot, finalScale);
                // Convert back to collider space
                newPoint = glm::vec3(glm::inverse(point.PointTransform) * glm::vec4(finalTrans, 1.0f));

                // Send the changes to the collider
                if (newPoint != point.SelectedPoint)
                {
                    if (currSelection.HasComponent<BoxCollider2DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateBoxCollider2D");

                        BoxCollider2DComponent& boxCollider = currSelection.GetComponent<BoxCollider2DComponent>();
                        boxCollider.SetPoint(glm::vec2(newPoint), point.SelectedName);
                    }
                    else if (currSelection.HasComponent<CircleCollider2DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateCircleCollider2D");

                        CircleCollider2DComponent& circleCollider = currSelection.GetComponent<CircleCollider2DComponent>();
                        if (point.SelectedName == "Top" || point.SelectedName == "Bottom")
                            newPoint.x = circleCollider.Offset.x;
                        else
                            newPoint.y = circleCollider.Offset.y;

                        circleCollider.SetPoint(glm::vec2(newPoint.x, newPoint.y), point.SelectedName);
                    }
                    else if (currSelection.HasComponent<PolygonCollider2DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdatePolygonCollider2D");

                        PolygonCollider2DComponent& polygonCollider = currSelection.GetComponent<PolygonCollider2DComponent>();
                        polygonCollider.SetPoint(std::stoi(point.SelectedName), glm::vec2(newPoint));
                    }
                    else if (currSelection.HasComponent<BoxCollider3DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateBoxCollider3D");

                        BoxCollider3DComponent& boxCollider = currSelection.GetComponent<BoxCollider3DComponent>();
                        boxCollider.SetPoint(point.SelectedName, newPoint);
                    }
                    else if (currSelection.HasComponent<SphereCollider3DComponent>())
                    {
                        DBT_PROFILE_SCOPE("Debutant::ManipulatePhysicsGizmos::UpdateSphereCollider3D");

                        SphereCollider3DComponent& sphereCollider = currSelection.GetComponent<SphereCollider3DComponent>();
                        sphereCollider.SetPoint(point.SelectedName, newPoint);
                    }
                }

                point.SelectedPoint = newPoint;
            }
        }
    }
}