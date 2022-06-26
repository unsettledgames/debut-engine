#include <Debut/dbtpch.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>
#include <Debut/Core/Instrumentor.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>


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
		DBT_PROFILE_FUNCTION();

		if (s_Data.Batches.find(meshComponent.Material) == s_Data.Batches.end())
			AddBatch(meshComponent.Material);

		Ref<Mesh> mesh;
		Ref<Material> material;

		{
			DBT_PROFILE_SCOPE("Renderer3D::GetMesh");
			mesh = AssetManager::Request<Mesh>(meshComponent.Mesh);
		}

		{
			DBT_PROFILE_SCOPE("Renderer3D::GetMaterial");
			material = AssetManager::Request<Material>(meshComponent.Material);
		}

		RenderBatch3D* currBatch = &s_Data.Batches[meshComponent.Material];

		// Send data to buffers
		{
			DBT_PROFILE_SCOPE("Renderer3D::PushData");
			currBatch->Buffers["Position"]->PushData(mesh->GetPositions().data(), sizeof(float) * mesh->GetPositions().size());

			// Add indices
			// This kinda sucks, the vector is reallocated every time a mesh is submitted (100 * 60 = 6000 times per second to be optimist)
			uint32_t currIndicesSize = currBatch->Indices.size();
			currBatch->Indices.resize(currBatch->Indices.size() + mesh->GetIndices().size());
			memcpy(currBatch->Indices.data() + currIndicesSize, mesh->GetIndices().data(), mesh->GetIndices().size() * sizeof(int));
		}
	}

	void Renderer3D::EndScene()
	{
		Flush();
	}

	void Renderer3D::Flush()
	{
		DBT_PROFILE_FUNCTION();

		for (auto& batch : s_Data.Batches)
		{
			batch.second.Material->Use(s_Data.CameraTransform);

			// Setup buffers
			for (auto& buffer : batch.second.Buffers)
				buffer.second->SubmitData();
			batch.second.IndexBuffer->SetData(batch.second.Indices.data(), batch.second.Indices.size());

			// Issue draw call
			RenderCommand::DrawIndexed(batch.second.VertexArray, batch.second.Indices.size());
			batch.second.Material->Unuse();

			// Clear buffers
			batch.second.Indices.clear();
		}
	}

	void Renderer3D::Shutdown()
	{
		// Delete all batches buffers
	}

	void Renderer3D::AddBatch(const UUID& id)
	{
		DBT_PROFILE_FUNCTION();

		if (s_Data.Batches.size() == s_Data.MaxBatches)
		{
			Log.CoreFatal("Renderer3D batch amount exceeded {0}", s_Data.MaxBatches);
			return;
		}

		RenderBatch3D newBatch;

		// Add material
		newBatch.Material = AssetManager::Request<Material>(id);

		// Setup buffers
		newBatch.Buffers["Position"] = VertexBuffer::Create(s_Data.StartupBufferSize);
		newBatch.Buffers["Position"]->SetLayout({ { ShaderDataType::Float, "a_Position", false } });

		newBatch.Buffers["Normals"] = VertexBuffer::Create(s_Data.StartupBufferSize);
		newBatch.Buffers["Normals"]->SetLayout({ { ShaderDataType::Float, "a_Normal", false } });

		newBatch.Buffers["Tangents"] = VertexBuffer::Create(s_Data.StartupBufferSize);
		newBatch.Buffers["Tangents"]->SetLayout({ { ShaderDataType::Float, "a_Tangent", false } });

		newBatch.Buffers["Bitangents"] = VertexBuffer::Create(s_Data.StartupBufferSize);
		newBatch.Buffers["Bitangents"]->SetLayout({ { ShaderDataType::Float, "a_Bitangent", false } });

		newBatch.Buffers["TexCoords0"] = VertexBuffer::Create(s_Data.StartupBufferSize);
		newBatch.Buffers["TexCoords0"]->SetLayout({ { ShaderDataType::Float, "a_TexCoords0", false } });

		// Create and configure vertex array
		newBatch.VertexArray = VertexArray::Create();
		newBatch.IndexBuffer = IndexBuffer::Create();
		for (auto& buffer : newBatch.Buffers)
		{
			newBatch.VertexArray->AddVertexBuffer(buffer.second);
		}
		newBatch.VertexArray->AddIndexBuffer(newBatch.IndexBuffer);
		
		// Add batch
		s_Data.Batches[newBatch.Material->GetID()] = newBatch;
	}
}