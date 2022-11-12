#include <Debut/dbtpch.h>

#include <Debut/Rendering/Structures/Frustum.h>
#include <Debut/Rendering/Camera.h>
#include <Debut/Utils/MathUtils.h>

namespace Debut
{
	Frustum::Frustum(const Camera& camera)
	{
		UpdateFrustum(camera);
	}

	void Frustum::UpdateFrustum(const Camera& camera)
	{
		float farDistance = camera.GetFarPlane();
		float nearDistance = camera.GetNearPlane();
		float fov = camera.GetFov();
		float aspectRatio = camera.GetAspectRatio();

		float wNear, wFar, hNear, hFar;
		glm::vec3 farCenter, nearCenter;
		glm::vec2 farHalfSize, nearHalfSize;
		glm::vec3 forward;
		glm::mat4 cameraTransform = glm::inverse(camera.GetView());

		// Near / far size
		hNear = 2.0f * glm::tan(fov / 2.0f) * nearDistance;
		hFar = 2.0f * glm::tan(fov / 2.0f) * farDistance;
		wNear = hNear * aspectRatio;
		wFar = hFar * aspectRatio;

		nearHalfSize = { wNear / 2.0f, hNear / 2.0f };
		farHalfSize = { wFar / 2.0f, hFar / 2.0f };

		// Directions
		forward = glm::vec3(0.0f, 0.0f, 1.0f);

		nearCenter = forward * nearDistance;
		farCenter = forward * farDistance;

		glm::vec3* vertices[8] = { &m_Far.TopLeft, &m_Far.TopRight, &m_Far.BottomLeft, &m_Far.BottomRight,
									&m_Near.TopLeft,&m_Near.TopRight,&m_Near.BottomLeft,&m_Near.BottomRight};
		glm::vec2 halfSizes[2] = { farHalfSize, nearHalfSize };
		glm::vec3 centers[2] = { farCenter, nearCenter };
		int xSigns[4] = { -1, 1, -1, 1 };
		int ySigns[4] = { 1, 1, -1, -1 };

		for (uint32_t i = 0; i < 8; i++)
		{
			*vertices[i] = {
				centers[i / 4].x + halfSizes[i / 4].x * xSigns[i % 4],
				centers[i / 4].y + halfSizes[i / 4].y * xSigns[i % 4],
				centers[i / 4].z
			};
			*vertices[i] = cameraTransform * glm::vec4(*vertices[i], 1.0f);
		}
	}

	std::vector<glm::vec3> Frustum::GetWorldViewPoints(const Camera& camera)
	{
		glm::mat4 inverse = glm::inverse(camera.GetProjection() * camera.GetView());

		std::vector<glm::vec3> ret;
		for (uint32_t x = 0; x < 2; x++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t z = 0; z < 2; z++)
				{
					glm::vec4 point = inverse * glm::vec4(2.0f * x, 2.0f * y, 2.0f * z, 1.0f);
					ret.push_back(point / point.w);
				}
			}
		}
		return ret;
	}

	std::vector<glm::vec3> Frustum::GetPoints()
	{
		std::vector<glm::vec3> ret = {
			m_Near.TopRight, m_Near.TopLeft, m_Near.BottomLeft, m_Near.BottomRight,
			m_Far.TopRight, m_Far.TopLeft, m_Far.BottomLeft, m_Far.BottomRight
		};
		return ret;
	}

}