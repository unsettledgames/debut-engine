#pragma once

#include "Debut/Rendering/Camera.h"

namespace Debut
{
	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		SceneCamera(ProjectionType type, float cameraNear, float cameraFar, float size, float aspectRatio);
		~SceneCamera() = default;

		void SetOrthographic(float size, float nearPlane, float farPlane);
		void SetPerspective(float fov, float nearPlane, float farPlane);
		void SetProjectionType(ProjectionType type);
		void SetViewportSize(uint32_t width, uint32_t height);

		float GetOrthoSize() const { return m_OrthographicSize; }
		float GetFOV() const { return m_FOV; }

		void SetOrthoSize(float val) { m_OrthographicSize = val; RecalculateProjection(); }
		void SetFOV(float val) { m_FOV = val; RecalculateProjection(); }

		static ProjectionType StringToProjType(const char* string);
	
	protected:
		void RecalculateProjection();
	protected:
		float m_FOV = glm::radians(40.0f);
		float m_OrthographicSize = 10.0f;
	};
}

