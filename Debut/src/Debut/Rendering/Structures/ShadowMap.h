#pragma once

#include <Debut/Core/Core.h>
#include <glm/glm.hpp>

namespace Debut
{
	class FrameBuffer;

	class ShadowMap
	{
	public:
		ShadowMap(uint32_t width, uint32_t height);

		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }

		inline glm::mat4 GetMatrix() { return m_ViewProjection; }
		inline void SetMatrix(const glm::mat4& viewProj) { m_ViewProjection = viewProj; }

		void Bind();
		void BindAsTexture(uint32_t slot);
		void Unbind();

		uint32_t GetRendererID();

	private:
		uint32_t m_Width;
		uint32_t m_Height;

		glm::mat4 m_ViewProjection;
		Ref<FrameBuffer> m_FrameBuffer;
	};
}