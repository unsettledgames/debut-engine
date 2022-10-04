#pragma once

#include <glm/glm.hpp>

namespace Debut
{
	namespace TransformationUtils
	{
		static inline glm::vec3 WorldToScreenPos(glm::vec3 worldPos, glm::mat4 viewProj, glm::mat4 model, glm::vec4 viewport)
		{
			glm::vec4 tmp = viewProj * model * glm::vec4(worldPos, 1.0);
			tmp /= tmp.w;

			return ((tmp + 1.0f) / 2.0f) * glm::vec4(viewport.z, viewport.w, 1.0f, 1.0f) + glm::vec4(viewport.x, viewport.y, 1.0f, 1.0f);
		}
	}
}