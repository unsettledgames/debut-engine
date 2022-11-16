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

		m_View = cameraTransform;

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

		// Compute vertices
		glm::vec3 farTopLeft, farTopRight, farBottomLeft, farBottomRight, 
			nearTopLeft, nearTopRight, nearBottomLeft, nearBottomRight;
		glm::vec3* vertices[8] = { &farTopLeft, &farTopRight, &farBottomLeft, &farBottomRight,
									&nearTopLeft, &nearTopRight, &nearBottomLeft, &nearBottomRight };
		glm::vec2 halfSizes[2] = { farHalfSize, nearHalfSize };
		glm::vec3 centers[2] = { farCenter, nearCenter };
		int xSigns[4] = { -1, 1, -1, 1 };
		int ySigns[4] = { 1, 1, -1, -1 };

		for (uint32_t i = 0; i < 8; i++)
		{
			*vertices[i] = {
				centers[i / 4].x + halfSizes[i / 4].x * xSigns[i % 4],
				centers[i / 4].y + halfSizes[i / 4].y * ySigns[i % 4],
				centers[i / 4].z
			};
			*vertices[i] = m_View * glm::vec4(*vertices[i], 1.0f);
		}

		// Compute planes
		// Pos normal
		m_Far.Normal = glm::normalize(glm::cross(farTopRight - farTopLeft, farTopLeft - farBottomLeft));
		m_Far.Distance = glm::dot(farTopLeft, -m_Far.Normal);

		// Neg normal
		m_Near.Normal = glm::normalize(glm::cross(nearTopLeft - nearTopRight, nearTopRight - nearBottomRight));
		m_Near.Distance = glm::dot(nearTopLeft, -m_Near.Normal);

		// Pos normal
		m_Left.Normal = glm::normalize(glm::cross(nearTopLeft - farTopLeft, nearTopLeft - nearBottomLeft));
		m_Left.Distance = glm::dot(farTopLeft, -m_Left.Normal);

		// Neg normal
		m_Right.Normal = glm::normalize(glm::cross(farTopRight - nearTopRight, farTopRight - farBottomRight));
		m_Right.Distance = glm::dot(farTopRight, -m_Right.Normal);

		// Neg normal
		m_Top.Normal = glm::normalize(glm::cross(farTopRight - farTopLeft, farTopLeft - nearTopLeft));
		m_Top.Distance = glm::dot(farTopLeft, -m_Top.Normal);

		// Pos normal
		m_Bottom.Normal = glm::normalize(glm::cross(farBottomLeft - farBottomRight, farBottomLeft - nearBottomLeft));
		m_Bottom.Distance = glm::dot(farBottomLeft, -m_Bottom.Normal);
	}

	bool Frustum::TestAABB(const AABB& aabb, const glm::mat4& transform)
	{
		std::vector<Plane*> planes = { &m_Far, &m_Near, &m_Top, &m_Bottom, &m_Left, &m_Right };
		glm::vec3 boxRight = transform * glm::vec4(1, 0, 0, 1);
		glm::vec3 boxUp = transform * glm::vec4(0, 1, 0, 1);
		glm::vec3 boxBackwards = transform * glm::vec4(0, 0, 1, 1);

		// Compute oriented box vertices
		std::vector<glm::vec3> boxVertices;
		glm::vec3 extents[2] = { aabb.MinExtents, aabb.MaxExtents };
		uint32_t vertIdx = 0;
		boxVertices.resize(8);

		for (uint32_t x = 0; x < 2; x++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t z = 0; z < 2; z++)
				{
					boxVertices[vertIdx] = aabb.Center +
						((x - 0.5f) * 2.0f) * extents[x].x * boxRight +
						((y - 0.5f) * 2.0f) * extents[y].y * boxUp +
						((z - 0.5f) * 2.0f) * extents[z].z * boxBackwards;
					vertIdx++;
				}
			}
		}

		for (auto& plane : planes)
		{
			// Compute pVertex
			float minAngle = glm::radians(360.0f);
			glm::vec3 pVertex;
			
			for (uint32_t i=0; i<boxVertices.size(); i++)
			{
				for (uint32_t j = i + 1; j < boxVertices.size(); j++)
				{
					glm::vec3 diagonal = boxVertices[i] - boxVertices[j];
					glm::vec3 toChoose = boxVertices[i];
					float currAngle = glm::orientedAngle(diagonal, plane->Normal, glm::normalize(glm::cross(diagonal, plane->Normal)));

					if (currAngle > glm::pi<float>())
					{
						currAngle = glm::pi<float>() * 2 - currAngle;
						toChoose = boxVertices[j];
					}
					if (currAngle < minAngle)
					{
						pVertex = toChoose;
						minAngle = currAngle;
					}
				}
			}

			float distance = plane->SignedDistance(pVertex);
			if (std::fabs(distance - plane->Distance) < 0)
				return false;
		}

		return true;
	}


	std::vector<glm::vec3> Frustum::GetWorldViewPoints(const Camera& camera)
	{
		glm::mat4 inverse = glm::inverse(camera.GetProjection() * camera.GetView());

		std::vector<glm::vec3> ret;
		for (uint32_t x = 0; x < 2; ++x)
		{
			for (uint32_t y = 0; y < 2; ++y)
			{
				for (uint32_t z = 0; z < 2; ++z)
				{
					glm::vec4 point = inverse * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					ret.push_back(point / point.w);
				}
			}
		}
		return ret;
	}
}