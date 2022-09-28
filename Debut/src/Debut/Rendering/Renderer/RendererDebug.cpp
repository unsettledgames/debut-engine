#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>

#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>

namespace Debut
{
	RendererDebugStorage RendererDebug::s_Storage;

	void RendererDebug::Init()
	{
		RenderCommand::SetLineWidth(1.3f);
		RenderCommand::SetPointSize(3.0f);

		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float4, "a_Color", false }
		};

		s_Storage.LineVertexArray = VertexArray::Create();
		s_Storage.LineVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Storage.MaxLines * sizeof(LineVertex));
		s_Storage.LineVertexBuffer->SetLayout(layout);
		s_Storage.LineVertexBase = new LineVertex[s_Storage.MaxLines];

		s_Storage.PointVertexArray = VertexArray::Create();
		s_Storage.PointVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Storage.MaxPoints * sizeof(PointVertex));
		s_Storage.PointVertexBuffer->SetLayout(layout);
		s_Storage.PointVertexBase = new PointVertex[s_Storage.MaxPoints];


		s_Storage.LineShader = Shader::Create("assets\\shaders\\line.glsl");
		s_Storage.PointShader = Shader::Create("assets\\shaders\\point.glsl");
	}
}