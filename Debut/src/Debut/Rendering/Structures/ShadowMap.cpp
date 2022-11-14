#include <Debut/Rendering/Structures/ShadowMap.h>
#include <Debut/Rendering/Structures/Frustum.h>
#include <Debut/Rendering/Camera.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Debut
{
	ShadowMap::ShadowMap(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
	{
		FrameBufferSpecifications specs;
		specs.Attachments = { FrameBufferTextureFormat::Depth };
		specs.Width = m_Width;
		specs.Height = m_Height;

		m_FrameBuffer = FrameBuffer::Create(specs);
	}

	void ShadowMap::SetFromCamera(const Camera& camera, const glm::vec3& lightDirection)
	{
		Camera referenceCamera(glm::perspective(camera.GetFov(), camera.GetAspectRatio(), m_Near, m_Far));
		referenceCamera.SetView(camera.GetView());
		std::vector<glm::vec3> points = Frustum::GetWorldViewPoints(referenceCamera);

		float left, right, top, down, front, bottom;
		glm::vec2 xBounds = { std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };
		glm::vec2 yBounds = { xBounds.x, xBounds.y };
		glm::vec2 zBounds = { xBounds.x, xBounds.y };

		glm::vec3 lightPos;
		glm::vec3 cameraPos = glm::vec3(0.0f);

		// Set camera point of view
		for (auto point : points)
			cameraPos += point;
		cameraPos /= points.size();

		lightPos = cameraPos + glm::normalize(lightDirection) * m_DistanceFromCamera;

		// Use the camera forward instead of its position
		float zMult = 3.0f;
		m_View = glm::lookAt(lightPos, cameraPos, glm::vec3(0.0f, 1.0f, 0.0f));

		for (auto point : points)
		{
			point = m_View * glm::vec4(point, 1.0f);

			xBounds.x = std::min(xBounds.x, point.x);
			xBounds.y = std::max(xBounds.y, point.x);

			yBounds.x = std::min(yBounds.x, point.y);
			yBounds.y = std::max(yBounds.y, point.y);

			zBounds.x = std::min(zBounds.x, point.z);
			zBounds.y = std::max(zBounds.y, point.z);
		}

		// Pull near
		if (zBounds.x < 0)
			zBounds.x *= zMult;
		else
			zBounds.x /= zMult;
		// Push far
		if (zBounds.y < 0)
			zBounds.y /= zMult;
		else
			zBounds.y *= zMult;

		m_Projection = glm::ortho(xBounds.x, xBounds.y, yBounds.x, yBounds.y, zBounds.x, zBounds.y);
		m_ViewProjection = m_Projection * m_View;
	}

	void ShadowMap::Bind()
	{
		m_FrameBuffer->Bind();
	}

	void ShadowMap::BindAsTexture(uint32_t slot)
	{
		m_FrameBuffer->BindDepth(slot);
	}

	void ShadowMap::Unbind()
	{
		m_FrameBuffer->Unbind();
	}

	void ShadowMap::UnbindTexture(uint32_t slot)
	{
		m_FrameBuffer->UnbindDepth(slot);
	}

	uint32_t ShadowMap::GetRendererID()
	{
		return m_FrameBuffer->GetDepthAttachment();
	}
}