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

	RenderTexture::RenderTexture(FrameBufferSpecifications specs)
	{
		if (!specs.Valid)
			return;
		specs.Attachments = { {FrameBufferTextureFormat::Color} };

		m_PrevBuffer = FrameBuffer::Create(specs);
		m_NextBuffer = FrameBuffer::Create(specs);
		m_BasicFullScreen = AssetManager::Request<Shader>("assets\\shaders\\fullscreenquad.glsl");

		m_Target = RenderTexture::Create(specs.Width, specs.Height, nullptr, RenderTextureMode::Color);

		uint32_t currHeight = specs.Height;
		uint32_t currWidth = specs.Width;
		uint32_t flag = std::min<uint32_t>(currHeight, currWidth);

		uint32_t i = 0;
		while (flag > 0)
		{
			FrameBufferSpecifications specs;

			specs.Width = currWidth;
			specs.Height = currHeight;
			specs.Attachments = { {FrameBufferTextureFormat::Color} };
			
			m_DownscaledBuffers[i].resize(2);
			m_DownscaledBuffers[i][0] = FrameBuffer::Create(specs);
			m_DownscaledBuffers[i][1] = FrameBuffer::Create(specs);

			i++;
			currWidth /= 2;
			currHeight /= 2;
			flag /= 2;
		}
	}

	void RenderTexture::Resize(uint32_t x, uint32_t y)
	{
		m_FrameBuffer->Resize(x, y);

		for (uint32_t i = 0; i < m_DownscaledBuffers.size(); i++)
		{
			m_DownscaledBuffers[i][0]->Resize(x, y);
			m_DownscaledBuffers[i][1]->Resize(x, y);
		}
	}

	void RenderTexture::Draw(Ref<FrameBuffer> startBuffer, Ref<Shader> startShader, Ref<PostProcessingStack> postProcessing)
	{
		DBT_PROFILE_SCOPE("Fullscreen::Draw");

		m_Target->SetSourceBuffer(startBuffer);

		m_NextBuffer->Bind();
		m_Target->Draw(startShader);
		m_NextBuffer->Unbind();

		Ref<FrameBuffer> tmp = m_NextBuffer;
		m_NextBuffer = m_PrevBuffer;
		m_PrevBuffer = tmp;

		if (postProcessing == nullptr)
			return;

		std::unordered_map<std::string, ShaderUniform> props;

		for (auto& volume : postProcessing->GetVolumes())
		{
			if (volume.RuntimeShader == nullptr || !volume.Enabled)
				continue;
			props.clear();
			props = volume.Properties;

			
			switch (volume.Type)
			{
			case PostProcessingEffect::Custom:
				m_Target->SetSourceBuffer(m_PrevBuffer);
				m_NextBuffer->Bind();
				m_Target->Draw(volume.RuntimeShader, volume.Properties);
				m_NextBuffer->Unbind();

				tmp = m_NextBuffer;
				m_NextBuffer = m_PrevBuffer;
				m_PrevBuffer = tmp;
				break;
			case PostProcessingEffect::Blur:
				// Build kernel
				int kernelSize = std::get<int>(props["u_Size"].Data);
				if (kernelSize < 1)
				{
					kernelSize = 1;
					volume.Properties["u_Size"].Data = kernelSize;
				}

				uint32_t i = 0;
				while (i < std::log2(kernelSize))
				{
					// Generate kernel
					std::vector<float> kernel = MathUtils::ComputeGaussianKernel(kernelSize, kernelSize / (i + 1), 0.0f);
					// Set horizontal, draw
					props["u_Size"].Data = (int)kernelSize;
					props["u_Kernel"] = ShaderUniform("u_Kernel", ShaderDataType::FloatArray, kernel);

					m_Target->SetSourceBuffer(m_PrevBuffer);
					ShaderUniform::UniformData data = (int)0;
					m_DownscaledBuffers[i][0]->Bind();
						props["u_Vertical"] = ShaderUniform("u_Vertical", ShaderDataType::Int, data);
						m_Target->Draw(volume.RuntimeShader, props);
					m_DownscaledBuffers[i][0]->Unbind();

					// Set vertical, draw
					m_Target->SetSourceBuffer(m_DownscaledBuffers[i][0]);
					m_DownscaledBuffers[i][1]->Bind();
						props["u_Vertical"].Data = (int)1;
						m_Target->Draw(volume.RuntimeShader, props);
					m_DownscaledBuffers[i][1]->Unbind();

					m_PrevBuffer = m_DownscaledBuffers[i][1];
					kernelSize /= 2.0f;
					i++;
				}

				// Draw on big buffer
				m_Target->SetSourceBuffer(m_PrevBuffer);
				m_NextBuffer->Bind();
				m_Target->Draw(m_BasicFullScreen, props);
				m_NextBuffer->Unbind();

				m_PrevBuffer = m_NextBuffer;
				m_NextBuffer = m_DownscaledBuffers[0][0];
				break;
			}
		}
	}

	void RenderTexture::Draw(Ref<Shader> shader, std::unordered_map<std::string, ShaderUniform>& properties)
	{
		uint32_t texIndex = 0;
		// Main texture
		m_VertexArray->Bind();
		shader->Bind();
		shader->SetInt("u_Texture", texIndex);
		shader->SetFloat2("u_TextureSize", { m_FrameBuffer->GetSpecs().Width, m_FrameBuffer->GetSpecs().Height });

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
			case ShaderDataType::FloatArray:
			{
				auto& vec = std::get<std::vector<float>>(param.second.Data);
				shader->SetFloatArray(param.second.Name, vec.data(), vec.size()); 
				break;
			}
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

		if (m_Target != nullptr)
			m_Target->BindTexture();
		else
			BindTexture();

		RenderCommand::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());

		if (m_Target != nullptr)
			m_Target->UnbindTexture();
		else
			UnbindTexture();

		shader->Unbind();
		m_VertexArray->Unbind();
	}

	void RenderTexture::DrawOverlay(Ref<FrameBuffer> under, Ref<FrameBuffer> above, Ref<Shader> shader)
	{
		DBT_PROFILE_SCOPE("Fullscreen::Draw");

		m_Target->SetSourceBuffer(above);

		under->Bind();
		m_Target->Draw(shader);
		under->Unbind();
	}
}