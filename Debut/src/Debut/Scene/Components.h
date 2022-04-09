#pragma once

#include <glm/glm.hpp>

namespace Debut
{
	struct TransformComponent
	{
		glm::mat4 Transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& mat) : Transform(mat) {}

		operator const glm::mat4& () const { return Transform; }
		operator glm::mat4& () { return Transform; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color;

		SpriteRendererComponent() : Color(glm::vec4(1.0f)) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
	};
}