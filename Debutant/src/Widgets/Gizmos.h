#pragma once

#include <Debut/Scene/Entity.h>

struct ImVec2;

namespace Debut
{
	class Entity;
	class EditorCamera;

	struct PhysicsColliderSelection
	{
		Entity SelectedEntity;
		glm::vec3 SelectedPoint = { 0,0,0 };
		glm::mat4 PointTransform = glm::mat4(1.0);
		std::string SelectedName = "";

		bool Valid = false;
	};

	class Gizmos
	{
	public:
		enum class GizmoOperation { Translate = 0, Rotate, Scale, All };
		enum class GizmoMode { Local = 0, Global };

		Gizmos() = default;

		inline void Enable() { m_Enabled = true; }
		inline void Disable() { m_Enabled = false; }

		void ManipulateTransform(Entity& selection, const EditorCamera& camera, ImVec2& viewportSize, ImVec2& windowPos);
		void ManipulateCollider(Entity& selection, const EditorCamera& camera, ImVec2& viewportSize, ImVec2& windowPos,
			PhysicsColliderSelection& point);

		inline void SetOperation(const GizmoOperation& op) { m_GizmoOperation = op; }
		inline void SetMode(const GizmoMode& mode) { m_GizmoMode = mode; }

		inline GizmoOperation GetOperation() { return m_GizmoOperation; }
		inline GizmoMode GetMode() { return m_GizmoMode; }

	private:
		// State
		bool m_Enabled;
		GizmoOperation m_GizmoOperation = GizmoOperation::Translate;
		GizmoMode m_GizmoMode = GizmoMode::Local;
	};
}