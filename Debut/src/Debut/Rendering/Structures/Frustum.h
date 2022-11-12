#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Debut
{
	class Camera;

	struct Plane
	{
		glm::vec3 TopRight;
		glm::vec3 TopLeft;

		glm::vec3 BottomRight;
		glm::vec3 BottomLeft;
	};

	class Frustum
	{
	public:
		Frustum(const Camera& camera);

		void UpdateFrustum(const Camera& camera);
		static std::vector<glm::vec3> GetWorldViewPoints(const Camera& camera);

		std::vector<glm::vec3> GetPoints();

	private:
		Plane m_Far;
		Plane m_Near;
	};
}