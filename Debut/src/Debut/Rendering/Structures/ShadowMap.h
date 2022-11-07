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
		inline Ref<FrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
		inline float GetNear() { return m_Near; }
		inline float GetFar() { return m_Far; }

		inline void SetMatrix(const glm::mat4& viewProj) { m_ViewProjection = viewProj; }
		inline void SetNear(float cameraNear) { m_Near = cameraNear; }
		inline void SetFar(float cameraFar) { m_Far = cameraFar; }

		void Bind();
		void BindAsTexture(uint32_t slot);

		void Unbind();
		void UnbindTexture(uint32_t slot);

		uint32_t GetRendererID();

	private:
		uint32_t m_Width;
		uint32_t m_Height;

		glm::mat4 m_ViewProjection;
		float m_Near;
		float m_Far;

		Ref<FrameBuffer> m_FrameBuffer;
	};
}