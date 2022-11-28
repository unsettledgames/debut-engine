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

		inline float GetOrthoSize() const { return m_OrthographicSize; }
		inline float GetFOV() const { return m_FOV; }
		inline glm::vec2 GetOrthoBoundsX() const { return m_OrthoBoundsX; }
		inline glm::vec2 GetOrthoBoundsY() const { return m_OrthoBoundsY; }
		inline glm::vec2 GetOrthoBoundsZ() const { return m_OrthoBoundsZ; }

		inline void SetOrthoSize(float val) { m_OrthographicSize = val; RecalculateProjection(); }
		inline void SetFOV(float val) { m_FOV = val; RecalculateProjection(); }
		inline void SetOrthoBoundsX(const glm::vec2& bounds) { m_OrthoBoundsX = bounds; }
		inline void SetOrthoBoundsY(const glm::vec2& bounds) { m_OrthoBoundsY = bounds; }
		inline void SetOrthoBoundsZ(const glm::vec2& bounds) { m_OrthoBoundsZ = bounds; }

		static ProjectionType StringToProjType(const char* string);
	
	protected:
		void RecalculateProjection();
	protected:
		float m_FOV = glm::radians(40.0f);
		float m_OrthographicSize = 10.0f;
		
		glm::vec2 m_OrthoBoundsX = {0,0};
		glm::vec2 m_OrthoBoundsY = {0,0};
		glm::vec2 m_OrthoBoundsZ = {0,0};
	};
}

