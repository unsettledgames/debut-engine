#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>

namespace Debut
{
	Renderer3DStorage Renderer3D::s_Data;

	void Renderer3D::Init()
	{
		DBT_PROFILE_FUNCTION();

		// Reserve space for the textures
		s_Data.Textures.resize(s_Data.MaxTextures);
	}

	void Renderer3D::BeginScene(Camera& camera, glm::mat4& transform)
	{
		s_Data.CameraTransform = camera.GetProjection() * glm::inverse(transform);
	}

	void Renderer3D::DrawModel(const MeshRendererComponent& model, const glm::mat4& transform)
	{
		if (s_Data.Batches.find(model.Material) == s_Data.Batches.end())
			AddBatch(model.Material);

		Ref<Mesh> mesh = AssetManager::Request<Mesh>(model.Mesh);
	}

	void Renderer3D::EndScene()
	{

	}

	void Renderer3D::Flush()
	{

	}

	void Renderer3D::Shutdown()
	{
		for (auto& batch : s_Data.Batches)
			delete[] batch.second.BufferBase;
	}

	void Renderer3D::AddBatch(const UUID& id)
	{
		if (s_Data.Batches.size() == s_Data.MaxBatches)
		{
			Log.CoreFatal("Renderer3D batch amount exceeded {0}", s_Data.MaxBatches);
			return;
		}

		RenderBatch3D newBatch;

		newBatch.BufferBase = new MeshVertex[s_Data.MaxVerticesPerBatch];
		newBatch.BufferPtr = newBatch.BufferBase;
		newBatch.VertexBuffer = VertexBuffer::Create(s_Data.MaxVerticesPerBatch);
		newBatch.IndexBuffer = IndexBuffer::Create(s_Data.MaxVerticesPerBatch);

		newBatch.Material = AssetManager::Request<Material>(id);
	}
}