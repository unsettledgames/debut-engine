#include "Debut/dbtpch.h"

#include "glm/gtc/matrix_transform.hpp"
#include "SceneCamera.h"

namespace Debut
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	SceneCamera::~SceneCamera()
	{

	}

	void SceneCamera::SetProjectionType(ProjectionType type)
	{
		if (type == ProjectionType::Perspective)
			SetPerspective(m_PerspectiveFOV, m_PerspectiveNear, m_PerspectiveFar);
		else
			SetOrthographic(m_OrthographicSize, m_OrthographicNear, m_OrthographicFar);
	}

	void SceneCamera::SetOrthographic(float size, float nearPlane, float farPlane)
	{
		m_ProjectionType = ProjectionType::Orthographic;

		m_OrthographicSize = size;
		m_OrthographicFar = farPlane;
		m_OrthographicNear = nearPlane;
		
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float fov, float nearPlane, float farPlane)
	{
		m_ProjectionType = ProjectionType::Perspective;

		m_PerspectiveFOV = fov;
		m_PerspectiveFar = farPlane;
		m_PerspectiveNear = nearPlane;

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
			m_ProjectionMatrix = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthoLeft = -0.5f * m_OrthographicSize * m_AspectRatio;
			float orthoRight = 0.5f * m_OrthographicSize * m_AspectRatio;
			float orthoBottom = -0.5f * m_OrthographicSize;
			float orthoTop = 0.5f * m_OrthographicSize;

			m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}
	}
}