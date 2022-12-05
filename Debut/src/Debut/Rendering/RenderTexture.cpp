#include <Debut/dbtpch.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/RenderTexture.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Renderer/RendererAPI.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>

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

	void RenderTexture::Draw(Ref<Shader> shader, std::unordered_map<std::string, ShaderUniform> params)
	{
		uint32_t texIndex = 0;
		// Main texture
		m_VertexArray->Bind();
		shader->Bind();
		shader->SetInt("u_Texture", texIndex);

		for (auto& param : params)
		{
			switch (param.second.Type)
			{
			case ShaderDataType::Float:		shader->SetFloat(param.second.Name, param.second.Data.Float); break;
			case ShaderDataType::Float2:	shader->SetFloat2(param.second.Name, param.second.Data.Vec2);break;
			case ShaderDataType::Float3:	shader->SetFloat3(param.second.Name, param.second.Data.Vec3);break;
			case ShaderDataType::Float4:	shader->SetFloat4(param.second.Name, param.second.Data.Vec4);break;
			case ShaderDataType::Int:		shader->SetInt(param.second.Name, param.second.Data.Int); break; 
			case ShaderDataType::Bool:		shader->SetBool(param.second.Name, param.second.Data.Bool); break;
			case ShaderDataType::Sampler2D: 
			{
				if (param.second.Data.Texture != 0)
				{
					texIndex++;
					Ref<Texture2D> texture = AssetManager::Request<Texture2D>(param.second.Data.Texture);
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
}