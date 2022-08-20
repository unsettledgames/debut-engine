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
		// Create generic VertexArray and IndexBuffer
		s_Data.VertexArray = VertexArray::Create();
		s_Data.IndexBuffer = IndexBuffer::Create();

		// Attach buffers to VertexArray
		ShaderDataType types[] = { ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2 };
		std::string attribNames[] = { "a_Position", "a_Normal", "a_Tangent", "a_Bitangent", "a_TexCoords0" };
		std::string names[] = { "Positions", "Normals", "Tangents", "Bitangents", "TexCoords0" };
		
		for (uint32_t i = 0; i < sizeof(types); i++)
		{
			s_Data.VertexBuffers[names[i]] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
			s_Data.VertexBuffers[names[i]]->SetLayout({ {types[i], attribNames[i], false} });
			s_Data.VertexArray->AddVertexBuffer(s_Data.VertexBuffers[names[i]]);
		}
		s_Data.VertexArray->AddIndexBuffer(s_Data.IndexBuffer);
	}

	void Renderer3D::BeginScene(Camera& camera, Ref<Skybox> skybox, glm::mat4& transform)
	{
		s_Data.CameraTransform = camera.GetProjection() * glm::inverse(transform);

		// Draw the skybox
		if (skybox != nullptr)
		{
			Ref<Material> skyboxMaterial = AssetManager::Request<Material>(skybox->GetMaterial());
			glm::mat4 skyboxTransform;
			if (camera.GetProjectionType() == Camera::ProjectionType::Perspective)
				skyboxTransform = camera.GetProjection() * glm::inverse(glm::mat4(glm::mat3(transform)));
			else
				skyboxTransform = glm::perspective(90.0f, camera.GetAspectRatio(), camera.GetNearPlane(), camera.GetFarPlane());
			
			skybox->Bind();
			skyboxMaterial->Use(skyboxTransform);

			std::vector<float>& positions = skybox->GetMesh().GetPositions();
			std::vector<int>& indices = skybox->GetMesh().GetIndices();
			s_Data.VertexBuffers["Positions"]->SetData(positions.data(), positions.size() * sizeof(float));
			s_Data.IndexBuffer->SetData(indices.data(), indices.size());

			RenderCommand::DrawIndexed(s_Data.VertexArray, indices.size());

			skybox->Unbind();
			skyboxMaterial->Unuse();

			s_Data.IndexBuffer->SetData("", 0);
			s_Data.VertexBuffers["Positions"]->SetData("", 0);
		}
	}

	void Renderer3D::DrawModel(const MeshRendererComponent& meshComponent, const glm::mat4& transform)
	{
		DBT_PROFILE_FUNCTION();

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

		if (mesh == nullptr)
		{
			Log.CoreError("Couldn't find mesh to render. Did you reimport and overwrite the model?");
			return;
		}
		if (material == nullptr)
		{
			Log.CoreError("Couldn't find material to render with. Did you reimport and overwrite the model?");
			return;
		}

		// Instanced rendering if the MeshRenderer is instanced
		if (meshComponent.Instanced)
		{
			if (s_Data.Batches.find(meshComponent.Material) == s_Data.Batches.end())
				AddBatch(meshComponent.Material);
			RenderBatch3D* currBatch = s_Data.Batches[meshComponent.Material];

			// Send data to buffers
			{
				DBT_PROFILE_SCOPE("Renderer3D::PushData");
				currBatch->Buffers["Positions"]->PushData(mesh->GetPositions().data(), sizeof(float) * mesh->GetPositions().size());
				
				if (mesh->HasNormals())
					currBatch->Buffers["Normals"]->PushData(mesh->GetNormals().data(), sizeof(float) * mesh->GetNormals().size());
				if (mesh->HasTangents())
					currBatch->Buffers["Tangents"]->PushData(mesh->GetTangents().data(), sizeof(float) * mesh->GetTangents().size());
				if (mesh->HasBitangents())
					currBatch->Buffers["Bitangents"]->PushData(mesh->GetBitangents().data(), sizeof(float) * mesh->GetBitangents().size());

				// Add indices
				uint32_t currIndicesSize = currBatch->Indices.size();
				currBatch->Indices.resize(currBatch->Indices.size() + mesh->GetIndices().size());
				memcpy(currBatch->Indices.data() + currIndicesSize, mesh->GetIndices().data(), mesh->GetIndices().size() * sizeof(int));
			}
		}
		// Just draw the model otherwise
		else
		{
			{
				DBT_PROFILE_SCOPE("DrawModel::SetDataAndIndices");
				std::vector<float>& positions = mesh->GetPositions();
				std::vector<int>& indices = mesh->GetIndices();
				s_Data.VertexBuffers["Positions"]->SetData(positions.data(), positions.size() * sizeof(float));
				s_Data.IndexBuffer->SetData(indices.data(), indices.size());

				if (mesh->HasNormals())
				{
					std::vector<float>& normals = mesh->GetNormals();
					s_Data.VertexBuffers["Normals"]->SetData(normals.data(), normals.size() * sizeof(float));
				}

				if (mesh->HasTangents())
				{
					std::vector<float>& tangents = mesh->GetTangents();
					s_Data.VertexBuffers["Tangents"]->SetData(tangents.data(), tangents.size() * sizeof(float));
				}

				if (mesh->HasBitangents())
				{
					std::vector<float>& bitangents = mesh->GetBitangents();
					s_Data.VertexBuffers["Bitangents"]->SetData(bitangents.data(), bitangents.size() * sizeof(float));
				}
			}
			
			{
				DBT_PROFILE_SCOPE("DrawModel::UseMaterial");
				material->SetMat4("u_Transform", transform);
				material->Use(s_Data.CameraTransform);
			}
			
			{
				DBT_PROFILE_SCOPE("DrawModel::DrawIndexed");
				RenderCommand::DrawIndexed(s_Data.VertexArray, mesh->GetIndices().size());
			}
		}
	}

	void Renderer3D::DrawModel(Mesh& mesh, Material& material, const glm::mat4& transform, bool instanced /* = false*/)
	{
		DBT_PROFILE_FUNCTION();

		// Instanced rendering if the MeshRenderer is instanced
		if (instanced)
		{
			if (s_Data.Batches.find(material.GetID()) == s_Data.Batches.end())
				AddBatch(material.GetID());
			RenderBatch3D* currBatch = s_Data.Batches[material.GetID()];

			// Send data to buffers
			{
				DBT_PROFILE_SCOPE("Renderer3D::PushData");
				currBatch->Buffers["Positions"]->PushData(mesh.GetPositions().data(), sizeof(float) * mesh.GetPositions().size());

				if (mesh.HasNormals())
					currBatch->Buffers["Normals"]->PushData(mesh.GetNormals().data(), sizeof(float) * mesh.GetNormals().size());
				if (mesh.HasTangents())
					currBatch->Buffers["Tangents"]->PushData(mesh.GetTangents().data(), sizeof(float) * mesh.GetTangents().size());
				if (mesh.HasBitangents())
					currBatch->Buffers["Bitangents"]->PushData(mesh.GetBitangents().data(), sizeof(float) * mesh.GetBitangents().size());

				// Add indices
				// This kinda sucks, the vector is reallocated every time a mesh is submitted (100 * 60 = 6000 times per second to be optimist)
				uint32_t currIndicesSize = currBatch->Indices.size();
				currBatch->Indices.resize(currBatch->Indices.size() + mesh.GetIndices().size());
				memcpy(currBatch->Indices.data() + currIndicesSize, mesh.GetIndices().data(), mesh.GetIndices().size() * sizeof(int));
			}
		}
		// Just draw the model otherwise
		else
		{
			{
				DBT_PROFILE_SCOPE("DrawModel::SetDataAndIndices");
				std::vector<float>& positions = mesh.GetPositions();
				std::vector<int>& indices = mesh.GetIndices();
				s_Data.VertexBuffers["Positions"]->SetData(positions.data(), positions.size() * sizeof(float));
				s_Data.IndexBuffer->SetData(mesh.GetIndices().data(), mesh.GetIndices().size());

				if (mesh.HasNormals())
				{
					std::vector<float>& normals = mesh.GetNormals();
					s_Data.VertexBuffers["Normals"]->SetData(normals.data(), normals.size() * sizeof(float));
				}

				if (mesh.HasTangents())
				{
					std::vector<float>& tangents = mesh.GetTangents();
					s_Data.VertexBuffers["Tangents"]->SetData(tangents.data(), tangents.size() * sizeof(float));
				}

				if (mesh.HasBitangents())
				{
					std::vector<float>& bitangents = mesh.GetBitangents();
					s_Data.VertexBuffers["Bitangents"]->SetData(bitangents.data(), bitangents.size() * sizeof(float));
				}
			}

			{
				DBT_PROFILE_SCOPE("DrawModel::UseMaterial");
				material.SetMat4("u_Transform", transform);
				material.Use(s_Data.CameraTransform);
			}

			{
				DBT_PROFILE_SCOPE("DrawModel::DrawIndexed");
				RenderCommand::DrawIndexed(s_Data.VertexArray, mesh.GetIndices().size());
			}
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
			batch.second->Material->Use(s_Data.CameraTransform);

			// Setup buffers
			for (auto& buffer : batch.second->Buffers)
				buffer.second->SubmitData();
			batch.second->IndexBuffer->SetData(batch.second->Indices.data(), batch.second->Indices.size());

			// Issue draw call
			RenderCommand::DrawIndexed(batch.second->VertexArray, batch.second->Indices.size());
			// Clear buffers
			batch.second->Indices.clear();
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

		RenderBatch3D* newBatch = new RenderBatch3D();

		// Add material
		newBatch->Material = AssetManager::Request<Material>(id);

		// Create and configure vertex array
		newBatch->VertexArray = VertexArray::Create();
		newBatch->IndexBuffer = IndexBuffer::Create();
		newBatch->VertexArray->AddIndexBuffer(newBatch->IndexBuffer);

		// Setup buffers
		newBatch->Buffers["Position"] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
		newBatch->Buffers["Position"]->SetLayout({ { ShaderDataType::Float3, "a_Position", false } });
		newBatch->VertexArray->AddVertexBuffer(newBatch->Buffers["Position"]);

		newBatch->Buffers["Normals"] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
		newBatch->Buffers["Normals"]->SetLayout({ { ShaderDataType::Float3, "a_Normal", false } });
		newBatch->VertexArray->AddVertexBuffer(newBatch->Buffers["Normals"]);

		newBatch->Buffers["Tangents"] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
		newBatch->Buffers["Tangents"]->SetLayout({ { ShaderDataType::Float3, "a_Tangent", false } });
		newBatch->VertexArray->AddVertexBuffer(newBatch->Buffers["Tangents"]);

		newBatch->Buffers["Bitangents"] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
		newBatch->Buffers["Bitangents"]->SetLayout({ { ShaderDataType::Float3, "a_Bitangent", false } });
		newBatch->VertexArray->AddVertexBuffer(newBatch->Buffers["Bitangents"]);

		newBatch->Buffers["TexCoords0"] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
		newBatch->Buffers["TexCoords0"]->SetLayout({ { ShaderDataType::Float2, "a_TexCoords0", false } });
		newBatch->VertexArray->AddVertexBuffer(newBatch->Buffers["TexCoords0"]);
		
		// Add batch
		s_Data.Batches[newBatch->Material->GetID()] = newBatch;
	}
}