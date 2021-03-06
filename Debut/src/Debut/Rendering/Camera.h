#pragma once

#include <glm/glm.hpp>

namespace Debut
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection) : m_ProjectionMatrix(projection) {}
		virtual ~Camera() {}

		glm::mat4 GetProjection() const { return m_ProjectionMatrix; }
		glm::mat4 GetView() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() { return m_ProjectionMatrix * glm::inverse(m_ViewMatrix); }
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
	};
}