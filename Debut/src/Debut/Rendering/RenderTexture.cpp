#include <Debut/dbtpch.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/RenderTexture.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Renderer/RendererAPI.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>

#include <Debut/Utils/MathUtils.h>
#include <Debut/Rendering/Resources/PostProcessing.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>
#include <Platform/OpenGL/OpenGLRenderTexture.h>>

namespace Debut
{
	Ref<RenderTexture> RenderTexture::Create(float width, float height, Ref<FrameBuffer> buffer, RenderTextureMode mode)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLRenderTexture>(width, height, buffer, mode);
		default:
			return nullptr;
		}
	}

	void RenderTexture::Draw(Ref<Shader> shader, std::unordered_map<std::string, ShaderUniform>& properties)
	{
		uint32_t texIndex = 0;
		// Main texture
		m_VertexArray->Bind();
		shader->Bind();
		shader->SetInt("u_Texture", texIndex);
		shader->SetFloat2("u_TextureSize", { m_Width, m_Height });

		for (auto& param : properties)
		{
			switch (param.second.Type)
			{
			case ShaderDataType::Float:		shader->SetFloat(param.second.Name, std::get<float>(param.second.Data)); break;
			case ShaderDataType::Float2:	shader->SetFloat2(param.second.Name, std::get<glm::vec2>(param.second.Data)); break;
			case ShaderDataType::Float3:	shader->SetFloat3(param.second.Name, std::get<glm::vec3>(param.second.Data)); break;
			case ShaderDataType::Float4:	shader->SetFloat4(param.second.Name, std::get<glm::vec4>(param.second.Data)); break;
			case ShaderDataType::Int:		shader->SetInt(param.second.Name, std::get<int>(param.second.Data)); break;
			case ShaderDataType::Bool:		shader->SetBool(param.second.Name, std::get<bool>(param.second.Data)); break;
			case ShaderDataType::Sampler2D:
			{
				if (std::get<UUID>(param.second.Data) != 0)
				{
					texIndex++;
					Ref<Texture2D> texture = AssetManager::Request<Texture2D>(std::get<UUID>(param.second.Data));
					texture->Bind(texIndex);
					shader->SetInt(param.second.Name, texIndex);
				}
				break;
			}
			default: break;
			}
		}

		Bind();

		RenderCommand::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());

		Unbind();
		shader->Unbind();

		m_VertexArray->Unbind();
	}

	void RenderTexture::Draw(Ref<Shader> shader, Ref<PostProcessingStack> postProcessingStack)
	{
		std::unordered_map<std::string, ShaderUniform> props;
		Draw(shader, props);

		if (postProcessingStack == nullptr)
			return;

		for (auto& volume : postProcessingStack->GetVolumes())
		{
			switch (volume.Type)
			{
			case PostProcessingEffect::Custom:
				Draw(volume.RuntimeShader, volume.Properties);
				break;
			case PostProcessingEffect::Blur:
				// Build kernel
				int kernelSize = std::get<int>(props["u_KernelSize"].Data);
				std::vector<float> kernel = MathUtils::ComputeGaussianKernel(kernelSize, 1.0f, (kernelSize * 2 + 1) / 2.0f);

				// Set horizontal, draw
				auto props = volume.Properties;
				ShaderUniform::UniformData data = false;
				props["u_Vertical"] = ShaderUniform("u_Vertical", ShaderDataType::Bool, data);
				Draw(volume.RuntimeShader, props);
				// Set vertical, draw
				props["u_Vertical"].Data = true;
				Draw(volume.RuntimeShader, props);
				break;
			}
		}
	}
}