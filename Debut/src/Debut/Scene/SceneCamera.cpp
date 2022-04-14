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

	void SceneCamera::SetOrthographic(float size, float nearPlane, float farPlane)
	{
		m_OrthographicSize = size;
		m_OrthographicFar = farPlane;
		m_OrthographicNear = nearPlane;
		
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		float orthoLeft = -0.5f * m_OrthographicSize * m_AspectRatio;
		float orthoRight = 0.5f * m_OrthographicSize * m_AspectRatio;
		float orthoBottom = -0.5f * m_OrthographicSize;
		float orthoTop = 0.5f * m_OrthographicSize;

		m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
	}
}