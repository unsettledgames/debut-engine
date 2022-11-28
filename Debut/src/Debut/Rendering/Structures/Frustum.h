#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

namespace Debut
{
	class SceneCamera;

	struct AABB
	{
		glm::vec3 Center;
		glm::vec3 MaxExtents;
		glm::vec3 MinExtents;

		glm::vec3 GetPositive(const glm::vec3& planeNormal)
		{

		}
	};

	struct Plane
	{
		glm::vec3 Normal;
		float Distance;

		float SignedDistance(const glm::vec3& point)
		{
			float dist = glm::dot(Normal, point) + Distance;
			return dist;
		}
	};

	class Frustum
	{
	public:
		Frustum() = default;
		Frustum(const SceneCamera& camera);

		void UpdateFrustum(const SceneCamera& camera);

		bool TestAABB(const AABB& aabb, const glm::mat4& transform);

		static std::vector<glm::vec3> GetWorldViewPoints(const SceneCamera& camera);

	private:
		Plane m_Far;
		Plane m_Near;
		Plane m_Top;
		Plane m_Bottom;
		Plane m_Left;
		Plane m_Right;

		glm::mat4 m_View;
	};
}