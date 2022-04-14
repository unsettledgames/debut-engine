#pragma once

#include "Debut/Renderer/Camera.h"

namespace Debut
{
	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		~SceneCamera();

		void SetOrthographic(float size, float nearPlane, float farPlane);
		void SetViewportSize(uint32_t width, uint32_t height);
	
	private:
		void RecalculateProjection();
	private:
		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f;
		float m_OrthographicFar = 1.0f;

		float m_AspectRatio;
	};
}

