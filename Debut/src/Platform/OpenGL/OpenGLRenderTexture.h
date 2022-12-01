#pragma once
#include <Debut/Rendering/RenderTexture.h>

namespace Debut
{
	class OpenGLRenderTexture : public RenderTexture
	{
	public:
		OpenGLRenderTexture(float width, float height, Ref<FrameBuffer> buffer, RenderTextureMode mode);

		virtual void Draw(Ref<Shader> shader, Ref< PostProcessingStack> postProcessingStack = nullptr) override;

		virtual void Bind() override;
		virtual void Unbind() override;
	};
}