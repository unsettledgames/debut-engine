#pragma once
#include <Debut/Rendering/RenderTexture.h>

namespace Debut
{
	class OpenGLRenderTexture : public RenderTexture
	{
	public:
		OpenGLRenderTexture(float width, float height, Ref<FrameBuffer> buffer);

		virtual void Draw(Ref<Shader> shader) override;

		virtual void Bind() override;
		virtual void Unbind() override;
	};
}