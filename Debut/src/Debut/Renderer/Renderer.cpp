#include "Debut/dbtpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

namespace Debut
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& va, const std::shared_ptr<Shader>& shader)
	{
		shader->Bind();
		va->Bind();

		shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);

		RenderCommand::DrawIndexed(va);
	}

	void Renderer::EndScene()
	{

	}
}