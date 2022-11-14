#include "Debut/dbtpch.h"

#include "glm/gtc/matrix_transform.hpp"
#include "SceneCamera.h"

namespace Debut
{
	SceneCamera::SceneCamera()
	{
		m_ProjectionType = ProjectionType::Orthographic;
		RecalculateProjection();
	}

	SceneCamera::~SceneCamera()
	{

	}

	void SceneCamera::SetProjectionType(ProjectionType type)
	{
		if (type == ProjectionType::Perspective)
			SetPerspective(m_FOV, m_NearPlane, m_FarPlane);
		else
			SetOrthographic(m_OrthographicSize, m_NearPlane, m_FarPlane);
	}

	void SceneCamera::SetOrthographic(float size, float nearPlane, float farPlane)
	{
		m_ProjectionType = ProjectionType::Orthographic;

		m_OrthographicSize = size;
		m_NearPlane = farPlane;
		m_FarPlane = nearPlane;
		
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float fov, float nearPlane, float farPlane)
	{
		m_ProjectionType = ProjectionType::Perspective;

		m_FOV = fov;
		m_FarPlane = farPlane;
		m_NearPlane = nearPlane;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_ProjectionMatrix = glm::perspective(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
		}
		else
		{
			float orthoLeft = -0.5f * m_OrthographicSize * m_AspectRatio;
			float orthoRight = 0.5f * m_OrthographicSize * m_AspectRatio;
			float orthoBottom = -0.5f * m_OrthographicSize;
			float orthoTop = 0.5f * m_OrthographicSize;

			m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_NearPlane, m_FarPlane);
		}
	}

	SceneCamera::ProjectionType SceneCamera::StringToProjType(const char* string)
	{
		if (std::string(string).compare("Perspective") == 0)
			return SceneCamera::ProjectionType::Perspective;
		
		return SceneCamera::ProjectionType::Orthographic;
	}
}