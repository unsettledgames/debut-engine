#pragma once

#include <Debut/Core/Core.h>
#include <glm/glm.hpp>

namespace Debut
{
	class FrameBuffer;

	class ShadowMap
	{
	public:
		ShadowMap(uint32_t width, uint32_t height, const glm::mat4& view, const glm::mat4& proj);

		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }

		void Bind();
		void Unbind();

		uint32_t GetRendererID();

	private:
		uint32_t m_Width;
		uint32_t m_Height;

		glm::mat4 m_ViewProjection;
		Ref<FrameBuffer> m_FrameBuffer;
	};
}