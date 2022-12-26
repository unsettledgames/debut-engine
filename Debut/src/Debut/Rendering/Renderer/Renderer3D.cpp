#include <Debut/dbtpch.h>


#include <Debut/Rendering/Texture.h>
#include <Debut/Scene/SceneCamera.h>

#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Resources/Skybox.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Rendering/Structures/ShadowMap.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Scene/Components.h>

#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Renderer/Renderer.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>
#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Core/Instrumentor.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>


namespace Debut
{
	Renderer3DStorage Renderer3D::s_Data;
	Renderer3DStats Renderer3D::s_Stats;
	Renderer3DStats Renderer3D::s_PrevStats;

	void Renderer3D::Init()
	{
		DBT_PROFILE_FUNCTION();

		// Reserve space for the textures
		s_Data.Textures.resize(s_Data.MaxTextures);
		// Create generic VertexArray and IndexBuffer
		s_Data.VertexArray = VertexArray::Create();
		s_Data.IndexBuffer = IndexBuffer::Create();

		{
			DBT_PROFILE_SCOPE("Renderer3D::Init::SetupBuffers");

			// Attach buffers to VertexArray
			ShaderDataType types[] = { ShaderDataType::Float3, ShaderDataType::Float4, ShaderDataType::Float3,
				ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2};
			std::string attribNames[] = { "a_Position", "a_Color", "a_Normal", "a_Tangent", "a_Bitangent", "a_TexCoords0"};
			std::string names[] = { "Positions", "Colors", "Normals", "Tangents", "Bitangents", "TexCoords0"};

			for (uint32_t i = 0; i < sizeof(types); i++)
			{
				s_Data.VertexBuffers[names[i]] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
				s_Data.VertexBuffers[names[i]]->SetLayout({ {types[i], attribNames[i], false} });
				s_Data.VertexArray->AddVertexBuffer(s_Data.VertexBuffers[names[i]]);
			}
		}
		
		s_Data.VertexArray->AddIndexBuffer(s_Data.IndexBuffer);
		{
			DBT_PROFILE_SCOPE("Renderer3D::Init::SetupDefaultMaterials");

			{
				DBT_PROFILE_SCOPE("CreateMaterials");

				s_Data.UntexturedMaterial = CreateRef<Material>();
				s_Data.VisualizeDepthmapMaterial = CreateRef<Material>();
				s_Data.DepthmapMaterial = CreateRef<Material>();
			}
			{
				DBT_PROFILE_SCOPE("SetShaders");
				s_Data.UntexturedMaterial->SetShader(AssetManager::Request<Shader>("assets\\shaders\\untextured.glsl"));
				s_Data.VisualizeDepthmapMaterial->SetShader(AssetManager::Request<Shader>("assets\\shaders\\depthvisualizer.glsl"));
				s_Data.DepthmapMaterial->SetShader(AssetManager::Request<Shader>("assets\\shaders\\depth.glsl"));
			}
		}
	}

	void Renderer3D::BeginScene(SceneCamera& camera, Ref<Skybox> skybox, const glm::mat4& cameraView,
		std::vector<LightComponent*>& lights, std::vector<ShaderUniform>& globalUniforms, 
		std::vector<Ref<ShadowMap>> shadowMaps)
	{
		// Reset storage
		s_Data.CameraView = camera.GetView();
		s_Data.CameraProjection = camera.GetProjection();
		s_Data.CameraNear = camera.GetNearPlane();
		s_Data.CameraFar = camera.GetFarPlane();
		s_Data.CameraFrustum = Frustum(camera);
		
		s_Data.Lights = lights;
		s_Data.GlobalUniforms = globalUniforms;
		s_Data.ShadowMaps = shadowMaps;		

		RenderCommand::DisableCulling();

		// Draw the skybox
		if (skybox != nullptr)
		{
			Ref<Material> skyboxMaterial = AssetManager::Request<Material>(skybox->GetMaterial());
			glm::mat4 skyboxTransform;
			if (camera.GetProjectionType() == Camera::ProjectionType::Perspective)
				skyboxTransform = camera.GetProjection() * glm::inverse(glm::mat4(glm::mat3(cameraView)));
			else
				skyboxTransform = glm::perspective(45.0f, 16.0f/9.0f, camera.GetNearPlane(), camera.GetFarPlane())
					* glm::inverse(glm::mat4(glm::mat3(cameraView)));
			
			skybox->Bind();
			skyboxMaterial->SetMat4("u_ViewProjection", skyboxTransform);
			skyboxMaterial->Use();

			std::vector<float>& positions = skybox->GetMesh().GetPositions();
			std::vector<int>& indices = skybox->GetMesh().GetIndices();
			s_Data.VertexBuffers["Positions"]->SetData(positions.data(), positions.size() * sizeof(float));
			s_Data.IndexBuffer->SetData(indices.data(), indices.size());

			RenderCommand::DrawIndexed(s_Data.VertexArray, indices.size());

			skybox->Unbind();
			skyboxMaterial->Unuse();
		}

		RenderCommand::EnableCulling();
		RenderCommand::CullBack();
	}

	void Renderer3D::DrawModel(const MeshRendererComponent& meshComponent, const glm::mat4& transform, int entityID)
	{
		DBT_PROFILE_FUNCTION();

		if (!s_Data.CameraFrustum.TestAABB(meshComponent.GetAABB(), transform))
			return;

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

		DrawModel(*mesh.get(), *material.get(), transform, entityID, meshComponent.Instanced);
	}

	void Renderer3D::DrawModel(Mesh& mesh, Material& material, const glm::mat4& transform, int entityID, bool instanced /* = false*/)
	{
		DBT_PROFILE_FUNCTION();
		Material materialToUse;
		Ref<VertexArray> vertexArray;

		{
			DBT_PROFILE_SCOPE("DrawModel::SendGeometry");
			vertexArray = mesh.GetVertexArray();

			if (s_Data.CurrentPass != RenderingPass::Shadow)
			{
				s_Stats.DrawCalls++;
				s_Stats.Triangles += mesh.GetNumVertices() / 3;
			}
			else
			{
				s_Stats.ShadowDrawCalls++;
				s_Stats.ShadowTriangles += mesh.GetNumVertices() / 3;
			}
		}

		{
			DBT_PROFILE_SCOPE("DrawModel::UseMaterial");

			if (s_Data.CurrentPass == RenderingPass::Shaded)
			{
				switch (Renderer::GetConfig().RenderingMode)
				{
				case RendererConfig::RenderingMode::Untextured:
					materialToUse = *s_Data.UntexturedMaterial.get();
					break;
				case RendererConfig::RenderingMode::Depth:
					materialToUse = *s_Data.VisualizeDepthmapMaterial.get();
					materialToUse.SetFloat("u_NearPlane", s_Data.CameraNear);
					materialToUse.SetFloat("u_FarPlane", s_Data.CameraFar);
					break;
				default:
					materialToUse = material;
					break;
				}

				SendLights(materialToUse);
				SendGlobals(materialToUse);
			}
			else
			{
				materialToUse = *s_Data.DepthmapMaterial.get();
				materialToUse.SetFloat("u_NearPlane", s_Data.CameraNear);
				materialToUse.SetFloat("u_FarPlane", s_Data.CameraFar);
			}

			if (Renderer::GetConfig().RenderingMode != RendererConfig::RenderingMode::None)
			{
				materialToUse.SetMat4("u_Transform", transform * mesh.GetTransform());
				materialToUse.SetMat4("u_ViewMatrix", s_Data.CameraView);
				materialToUse.SetMat4("u_ProjectionMatrix", s_Data.CameraProjection);

				materialToUse.Use();
				if (materialToUse.GetRuntimeShader() != nullptr)
				{
					materialToUse.GetRuntimeShader()->SetMat4("u_ViewProjection", s_Data.CameraProjection * s_Data.CameraView);
					materialToUse.GetRuntimeShader()->SetMat4("u_MVP", s_Data.CameraProjection * (s_Data.CameraView * transform));
					materialToUse.GetRuntimeShader()->SetMat4("u_NormalMatrix", glm::inverse(glm::transpose(transform)));
					materialToUse.GetRuntimeShader()->SetInt("u_EntityID", entityID);
				}

				if (s_Data.CurrentPass != RenderingPass::Shadow)
				{
					// Set shadowmaps
					for (uint32_t i = 0; i < s_Data.ShadowMaps.size(); i++)
					{
						std::stringstream ss;
						ss << "u_ShadowMaps[" << i << "]";

						if (materialToUse.GetRuntimeShader() != nullptr)
						{
							materialToUse.GetRuntimeShader()->SetMat4(ss.str() + ".LightMatrix", s_Data.ShadowMaps[i]->GetMatrix());
							materialToUse.GetRuntimeShader()->SetInt(ss.str() + ".Sampler", materialToUse.GetCurrentTextureSlot() + i);
							materialToUse.GetRuntimeShader()->SetFloat(ss.str() + ".Near", s_Data.ShadowMaps[i]->GetNear());
							materialToUse.GetRuntimeShader()->SetFloat(ss.str() + ".Far", s_Data.ShadowMaps[i]->GetFar());
							s_Data.ShadowMaps[i]->BindAsTexture(materialToUse.GetCurrentTextureSlot() + i);
						}
					}
				}
			}
		}

		{
			DBT_PROFILE_SCOPE("DrawModel::DrawIndexed");
			RenderCommand::DrawIndexed(vertexArray, mesh.GetNumIndices());
			materialToUse.Unuse();
			if (s_Data.CurrentPass != RenderingPass::Shadow)
				s_Data.ShadowMaps[0]->UnbindTexture(8);
		}

		if (Renderer::GetConfig().RenderWireframe && s_Data.CurrentPass == RenderingPass::Shaded)
			RendererDebug::DrawMesh(mesh.GetID(), glm::vec3(0.0f), transform, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void Renderer3D::EndScene()
	{
		Flush();

		if (Renderer::GetConfig().RenderWireframe)
			RendererDebug::EndScene();
	}

	void Renderer3D::BeginShadow(Ref<ShadowMap> shadowMap, SceneCamera& camera)
	{
		s_Data.CameraView = shadowMap->GetView();
		s_Data.CameraProjection = shadowMap->GetProjection();
		s_Data.CurrentPass = RenderingPass::Shadow;
		s_Data.CameraNear = shadowMap->GetNear();
		s_Data.CameraFar = shadowMap->GetFar();
		s_Data.CameraFrustum = Frustum(camera);

		s_Stats.NShadowPasses++;

		//RenderCommand::CullFront();
	}

	void Renderer3D::EndShadow()
	{
		s_Data.CurrentPass = RenderingPass::Shaded;
		//RenderCommand::CullBack();
	}

	void Renderer3D::Flush()
	{
		DBT_PROFILE_FUNCTION();

		for (auto& batch : s_Data.Batches)
		{
			batch.second->Material->SetMat4("u_ViewMatrix", s_Data.CameraView);
			batch.second->Material->SetMat4("u_ProjectionMatrix", s_Data.CameraProjection);
			batch.second->Material->SetMat4("u_ViewProjection", s_Data.CameraProjection * s_Data.CameraView);
			batch.second->Material->Use();

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

	void Renderer3D::SendLights(Material& material)
	{
		std::vector<PointLightComponent> pointLights;
		ShaderUniform::UniformData data;

		for (LightComponent* light : s_Data.Lights)
		{
			switch (light->Type)
			{
			case LightComponent::LightType::Directional:
			{
				DirectionalLightComponent* dirLight = static_cast<DirectionalLightComponent*>(light);

				data = dirLight->Direction;
				material.m_Uniforms["u_DirectionalLightDir"] = {
					ShaderUniform("u_DirectionalLightDir", ShaderDataType::Float3, data) };

				data = dirLight->Color;
				material.m_Uniforms["u_DirectionalLightCol"] = {
					ShaderUniform("u_DirectionalLightCol", ShaderDataType::Float3, data) };

				data = dirLight->Intensity;
				material.m_Uniforms["u_DirectionalLightIntensity"] = {
					ShaderUniform("u_DirectionalLightIntensity", ShaderDataType::Float, data) };
				break;
			}
			case LightComponent::LightType::Point:
			{
				PointLightComponent* pointLight = static_cast<PointLightComponent*>(light);
				pointLights.push_back(*pointLight);
				break;
			}
			}
		}

		// Send point lights
		for (uint32_t i = 0; i < pointLights.size(); i++)
		{
			std::stringstream lightName;
			lightName << "u_PointLights[" << i << "]";

			data = pointLights[i].Color;
			material.m_Uniforms[lightName.str() + ".Color"] = { ShaderUniform(lightName.str() + ".Color", ShaderDataType::Float3, data) };
			data = pointLights[i].Position;
			material.m_Uniforms[lightName.str() + ".Position"] = { ShaderUniform(lightName.str() + ".Position", ShaderDataType::Float3, data) };

			data = pointLights[i].Intensity;
			material.m_Uniforms[lightName.str() + ".Intensity"] = { ShaderUniform(lightName.str() + ".Intensity", ShaderDataType::Float, data) };
			data = pointLights[i].Radius;
			material.m_Uniforms[lightName.str() + ".Radius"] = { ShaderUniform(lightName.str() + ".Radius", ShaderDataType::Float, data) };
		}

		data = (int)pointLights.size();
		material.m_Uniforms["u_NPointLights"] = { ShaderUniform("u_NPointLights", ShaderDataType::Int, data) };
	}

	void Renderer3D::SendGlobals(Material& material)
	{
		for (auto& uniform : s_Data.GlobalUniforms)
			material.m_Uniforms[uniform.Name] = uniform;
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

		newBatch->Buffers["Colors"] = VertexBuffer::Create(s_Data.StartupBufferSize * sizeof(float), s_Data.MaxVerticesPerBatch * sizeof(float));
		newBatch->Buffers["Colors"]->SetLayout({ { ShaderDataType::Float4, "a_Color", false } });
		newBatch->VertexArray->AddVertexBuffer(newBatch->Buffers["Colors"]);

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

	void Renderer3D::ResetStats()
	{
		s_PrevStats = s_Stats;
		// Reset stats
		s_Stats.DrawCalls = 0;
		s_Stats.NShadowPasses = 0;
		s_Stats.Triangles = 0;
		s_Stats.ShadowDrawCalls = 0;
		s_Stats.ShadowTriangles = 0;
	}
}