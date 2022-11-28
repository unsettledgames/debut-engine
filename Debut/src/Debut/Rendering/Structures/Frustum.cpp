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
		float aspectRatio = camera.GetAspectRatio();

		glm::vec3 farCenter, nearCenter;
		glm::vec2 farHalfSize, nearHalfSize;
		glm::vec2 nearXBounds, nearYBounds, farXBounds, farYBounds;
		glm::vec3 forward;
		glm::vec3 centerOffset = { (camera.GetOrthoBoundsX().x + camera.GetOrthoBoundsX().y) / 2.0f,
			(camera.GetOrthoBoundsY().x + camera.GetOrthoBoundsY().y) / 2.0f,
			(camera.GetOrthoBoundsZ().x + camera.GetOrthoBoundsZ().y) / 2.0f };
		glm::mat4 cameraTransform = glm::inverse(camera.GetView());

		m_View = cameraTransform;

		if (camera.GetProjectionType() == Camera::ProjectionType::Perspective)
		{
			float fov = camera.GetFOV();
			float wNear, wFar, hNear, hFar;
			// Near / far size
			hNear = 2.0f * glm::tan(fov / 2.0f) * nearDistance;
			hFar = 2.0f * glm::tan(fov / 2.0f) * farDistance;
			wNear = hNear * aspectRatio;
			wFar = hFar * aspectRatio;

			nearXBounds = { -wNear / 2.0f, wNear / 2.0f };
			nearYBounds = { -hNear / 2.0f, hNear / 2.0f };
			farXBounds = { -wFar / 2.0f, wFar / 2.0f };
			farYBounds = { -hFar / 2.0f, hFar / 2.0f };
		}
		else
		{
			nearXBounds = camera.GetOrthoBoundsX();
			nearYBounds = camera.GetOrthoBoundsY();
			farXBounds = nearXBounds;
			farYBounds = nearYBounds;
			nearDistance = camera.GetOrthoBoundsZ().x;
			farDistance = camera.GetOrthoBoundsZ().y;
		}

		// Directions
		forward = glm::vec3(0.0f, 0.0f, -1.0f);

		nearCenter = forward * nearDistance;
		farCenter = forward * farDistance;

		// Compute vertices
		glm::vec3 farTopLeft, farTopRight, farBottomLeft, farBottomRight, 
			nearTopLeft, nearTopRight, nearBottomLeft, nearBottomRight;

		farTopLeft =		m_View * glm::vec4(farCenter + glm::vec3(farXBounds.x, farYBounds.y, 0.0f), 1.0f);
		farTopRight =		m_View * glm::vec4(farCenter + glm::vec3(farXBounds.y, farYBounds.y, 0.0f), 1.0f);
		farBottomLeft =		m_View * glm::vec4(farCenter + glm::vec3(farXBounds.x, farYBounds.x, 0.0f), 1.0f);
		farBottomRight =	m_View * glm::vec4(farCenter + glm::vec3(farXBounds.y, farYBounds.x, 0.0f), 1.0f);
		nearTopLeft =		m_View * glm::vec4(nearCenter + glm::vec3(nearXBounds.x, nearYBounds.y, 0.0f), 1.0f);
		nearTopRight =		m_View * glm::vec4(nearCenter + glm::vec3(nearXBounds.y, nearYBounds.y, 0.0f), 1.0f);
		nearBottomLeft =	m_View * glm::vec4(nearCenter + glm::vec3(nearXBounds.x, nearYBounds.x, 0.0f), 1.0f);
		nearBottomRight =	m_View * glm::vec4(nearCenter + glm::vec3(nearXBounds.y, nearYBounds.x, 0.0f), 1.0f);

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
		DBT_PROFILE_SCOPE("FrustumCulling::TestAABB");
		std::vector<Plane*> planes = { &m_Far, &m_Near, &m_Top, &m_Bottom, &m_Left, &m_Right };
		glm::vec3 boxRight = glm::normalize(transform * glm::vec4(1, 0, 0, 0));
		glm::vec3 boxUp = glm::normalize(transform * glm::vec4(0, 1, 0, 0));
		glm::vec3 boxBackwards = glm::normalize(transform * glm::vec4(0, 0, 1, 0));

		// Compute oriented box vertices
		glm::vec3 extents[2] = { aabb.MinExtents, aabb.MaxExtents };

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

			pVertex = transform * glm::vec4(aabb.Center + pVertex, 1.0f);

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