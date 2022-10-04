#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Debut
{
	namespace MathUtils
	{
		bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
		
		std::vector<uint32_t> Triangulate(const std::vector<glm::vec2>& vertices);
	}
}