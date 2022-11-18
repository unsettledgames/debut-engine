#include <Debut/dbtpch.h>

#include <Debut/Rendering/Structures/Frustum.h>
#include <Debut/Scene/SceneCamera.h>
#include <Debut/Utils/MathUtils.h>

namespace Debut
{
	Frustum::Frustum(const SceneCamera& camera)
	{
		UpdateFrustum(camera);
	}

	void Frustum::UpdateFrustum(const SceneCamera& camera)
	{
		float farDistance = camera.GetFarPlane();
		float nearDistance = camera.GetNearPlane();
		float fov = camera.GetFOV();
		float aspectRatio = camera.GetAspectRatio();

		float wNear, wFar, hNear, hFar;
		glm::vec3 farCenter, nearCenter;
		glm::vec2 farHalfSize, nearHalfSize;
		glm::vec3 forward;
		glm::mat4 cameraTransform = glm::inverse(camera.GetView());

		m_View = cameraTransform;

		if (camera.GetProjectionType() == Camera::ProjectionType::Perspective)
		{
			// Near / far size
			hNear = 2.0f * glm::tan(fov / 2.0f) * nearDistance;
			hFar = 2.0f * glm::tan(fov / 2.0f) * farDistance;
			wNear = hNear * aspectRatio;
			wFar = hFar * aspectRatio;

			nearHalfSize = { wNear / 2.0f, hNear / 2.0f };
			farHalfSize = { wFar / 2.0f, hFar / 2.0f };
		}
		else
		{
			float height = camera.GetOrthoSize();
			float width = height * camera.GetAspectRatio();

			nearHalfSize = { height / 2.0f, width / 2.0f };
			farHalfSize = nearHalfSize;
		}
		

		// Directions
		forward = glm::vec3(0.0f, 0.0f, -1.0f);

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
		m_Far.Normal = glm::normalize(glm::cross(farBottomLeft - farTopLeft, farTopRight - farTopLeft));
		m_Far.Distance = glm::dot(farTopLeft, -m_Far.Normal);

		m_Near.Normal = glm::normalize(glm::cross(nearBottomRight - nearTopRight, nearTopLeft - nearTopRight));
		m_Near.Distance = glm::dot(nearTopLeft, -m_Near.Normal);

		m_Left.Normal = glm::normalize(glm::cross(nearBottomLeft - nearTopLeft, farTopLeft - nearTopLeft));
		m_Left.Distance = glm::dot(farTopLeft, -m_Left.Normal);

		m_Right.Normal = glm::normalize(glm::cross(farBottomRight - farTopRight, nearTopRight - farTopRight));
		m_Right.Distance = glm::dot(farTopRight, -m_Right.Normal);

		m_Top.Normal = glm::normalize(glm::cross(farTopLeft - nearTopLeft, nearTopRight - nearTopLeft));
		m_Top.Distance = glm::dot(farTopLeft, -m_Top.Normal);

		m_Bottom.Normal = glm::normalize(glm::cross(nearBottomLeft - farBottomLeft, farBottomRight - farBottomLeft));
		m_Bottom.Distance = glm::dot(farBottomLeft, -m_Bottom.Normal);
	}

	bool Frustum::TestAABB(const AABB& aabb, const glm::mat4& transform)
	{
		std::vector<Plane*> planes = { &m_Far, &m_Near, &m_Top, &m_Bottom, &m_Left, &m_Right };
		glm::vec3 boxRight = glm::normalize(transform * glm::vec4(1, 0, 0, 0));
		glm::vec3 boxUp = glm::normalize(transform * glm::vec4(0, 1, 0, 0));
		glm::vec3 boxBackwards = glm::normalize(transform * glm::vec4(0, 0, 1, 0));

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
					boxVertices[vertIdx] = glm::vec3(
						((x - 0.5f) * 2.0f) * extents[x].x,
						((y - 0.5f) * 2.0f) * extents[y].y,
						((z - 0.5f) * 2.0f) * extents[z].z);
					vertIdx++;
				}
			}
		}

		for (auto& plane : planes)
		{
			// Compute pVertex
			glm::vec3 pVertex = glm::vec3(0.0f);
			glm::vec3 boxSpaceNormal = glm::normalize(glm::vec3(glm::dot(boxRight, plane->Normal), 
				glm::dot(boxUp, plane->Normal), glm::dot(boxBackwards, plane->Normal)));

			if (boxSpaceNormal.x > 0)
				pVertex.x = aabb.MaxExtents.x;
			else
				pVertex.x = aabb.MinExtents.x;

			if (boxSpaceNormal.y > 0)
				pVertex.y = aabb.MaxExtents.y;
			else
				pVertex.y = aabb.MinExtents.y;

			if (boxSpaceNormal.z > 0)
				pVertex.z = aabb.MaxExtents.z;
			else
				pVertex.z = aabb.MinExtents.z;

			pVertex = transform * glm::vec4(pVertex, 1.0f);

			// Add support to transformed objects (compute the pVertex keeping the transform in account)
			float distance = plane->SignedDistance(pVertex);
			if (distance < 0)
				return false;
		}

		return true;
	}


	std::vector<glm::vec3> Frustum::GetWorldViewPoints(const SceneCamera& camera)
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