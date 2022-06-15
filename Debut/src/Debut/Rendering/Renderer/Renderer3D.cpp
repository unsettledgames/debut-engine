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

	void Renderer3D::DrawModel(const MeshRendererComponent& meshComponent, const glm::mat4& transform)
	{
		if (s_Data.Batches.find(meshComponent.Material) == s_Data.Batches.end())
			AddBatch(meshComponent.Material);

		Ref<Mesh> mesh = AssetManager::Request<Mesh>(meshComponent.Mesh);
		Ref<Material> material = AssetManager::Request<Material>(meshComponent.Material);

		RenderBatch3D* currBatch = &s_Data.Batches[meshComponent.Material];

		// Ideally,
		currBatch->Buffers["Position"]->PushData(mesh->GetPositions().data(), sizeof(glm::vec3) * mesh->GetPositions().size());
		currBatch->Indices.insert(currBatch->Indices.end(), mesh->GetIndices().begin(), mesh->GetIndices().end());
	}

	void Renderer3D::EndScene()
	{
		Flush();
	}

	void Renderer3D::Flush()
	{
		for (auto& batch : s_Data.Batches)
		{
			batch.second.Material->Use(s_Data.CameraTransform);
			batch.second.Buffers["Position"]->SubmitData();
			batch.second.Material->Unuse();
		}
		// Delete the buffers for the previous batches
		// OPTIMIZABLE: leave the buffers, just clear them and reuse them if the material is used in the next frame (very likely)
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

		// Add material
		newBatch.Material = AssetManager::Request<Material>(id);

		// Setup buffers
		newBatch.BufferBase = new MeshVertex[s_Data.MaxVerticesPerBatch];
		newBatch.BufferPtr = newBatch.BufferBase;
		
		// Add batch
		s_Data.Batches[newBatch.Material->GetID()] = newBatch;
	}
}