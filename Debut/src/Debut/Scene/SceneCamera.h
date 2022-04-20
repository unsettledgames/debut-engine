#pragma once

#include "Debut/Renderer/Camera.h"

namespace Debut
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };
		SceneCamera();
		~SceneCamera();

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetOrthographic(float size, float nearPlane, float farPlane);
		void SetPerspective(float fov, float nearPlane, float farPlane);
		void SetProjectionType(ProjectionType type);
		void SetViewportSize(uint32_t width, uint32_t height);

		float GetOrthoNearClip() const { return m_OrthographicNear; }
		float GetOrthoFarClip() const { return m_OrthographicFar; }
		float GetOrthoSize() const { return m_OrthographicSize; }

		void SetOrthoSize(float val) { m_OrthographicSize = val; RecalculateProjection(); }
		void SetOrthoNearClip(float val) { m_OrthographicNear = val; RecalculateProjection(); }
		void SetOrthoFarClip(float val) { m_OrthographicFar = val; RecalculateProjection(); }


		float GetPerspNearClip() const { return m_PerspectiveNear; }
		float GetPerspFarClip() const { return m_PerspectiveFar; }
		float GetPerspFOV() const { return m_PerspectiveFOV; }

		void SetPerspFOV(float val) { m_PerspectiveFOV = val; RecalculateProjection(); }		
		void SetPerspNearClip(float val) { m_PerspectiveNear = val; RecalculateProjection(); }
		void SetPerspFarClip(float val) { m_PerspectiveFar = val; RecalculateProjection(); }

		
	
	private:
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_AspectRatio;
	};
}

