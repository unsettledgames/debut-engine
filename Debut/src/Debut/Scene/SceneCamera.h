#pragma once

#include "Debut/Rendering/Camera.h"

namespace Debut
{
	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		~SceneCamera();

		void SetOrthographic(float size, float nearPlane, float farPlane);
		void SetPerspective(float fov, float nearPlane, float farPlane);
		void SetProjectionType(ProjectionType type);
		void SetViewportSize(uint32_t width, uint32_t height);

		float GetOrthoSize() const { return m_OrthographicSize; }
		float GetPerspFOV() const { return glm::radians(m_FOV); }

		void SetOrthoSize(float val) { m_OrthographicSize = val; RecalculateProjection(); }
		void SetPerspFOV(float val) { m_FOV = val; RecalculateProjection(); }

		static ProjectionType StringToProjType(const char* string);
	
	private:
		void RecalculateProjection();
	private:
		float m_OrthographicSize = 10.0f;

		float m_AspectRatio;
	};
}

