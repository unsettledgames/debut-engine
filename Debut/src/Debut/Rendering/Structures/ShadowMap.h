#pragma once

#include <Debut/Core/Core.h>
#include <glm/glm.hpp>

namespace Debut
{
	class FrameBuffer;
	class SceneCamera;

	class ShadowMap
	{
	public:
		ShadowMap(uint32_t width, uint32_t height);

		inline uint32_t GetIndex() { return m_Index; }
		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }
		inline glm::mat4 GetMatrix() { return m_ViewProjection; }
		inline glm::mat4 GetView() { return m_View; }
		inline glm::mat4 GetProjection() { return m_Projection; }
		inline Ref<FrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
		inline float GetNear() { return m_Near; }
		inline float GetFar() { return m_Far; }

		inline void SetIndex(uint32_t idx) { m_Index = idx; }
		inline void SetMatrix(const glm::mat4& viewProj) { m_ViewProjection = viewProj; }
		inline void SetNear(float cameraNear) { m_Near = cameraNear; }
		inline void SetFar(float cameraFar) { m_Far = cameraFar; }
		inline void SetCameraDistance(float cameraDistance) { m_DistanceFromCamera = cameraDistance; }

		void SetFromCamera(const SceneCamera& camera, SceneCamera& outCamera, const glm::vec3& lightDirection);

		void Bind();
		void BindAsTexture(uint32_t slot);

		void Unbind();
		void UnbindTexture(uint32_t slot);

		uint32_t GetRendererID();

	private:
		uint32_t m_Index;
		uint32_t m_Width;
		uint32_t m_Height;

		glm::mat4 m_ViewProjection;
		glm::mat4 m_Projection;
		glm::mat4 m_View;

		float m_Near;
		float m_Far;
		float m_DistanceFromCamera;

		Ref<FrameBuffer> m_FrameBuffer;
	};
}