#pragma once

#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Scene/Components.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Camera.h>

namespace Debut
{
	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec3 Normal;
		glm::vec3 Tangent;
	};

	struct RenderBatch3D
	{
		MeshVertex* BufferBase;
		MeshVertex* BufferPtr;

		std::unordered_map<std::string, Ref<VertexBuffer>> Buffers;

		std::vector<int> Indices;
		Ref<VertexArray> VertexArray;
		Ref<Material> Material;
	};

	struct Renderer3DStorage
	{
		uint32_t MaxTextures = 32;
		uint32_t MaxBatches = 64;
		uint32_t MaxMeshesPerBatch = 16384;
		uint32_t MaxVerticesPerBatch = 360000;

		std::vector<Ref<Texture2D>> Textures;
		// One batch per Material
		std::unordered_map<UUID, RenderBatch3D> Batches;

		glm::mat4 CameraTransform;
	};

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(Camera& camera, glm::mat4& transform);
		static void EndScene();
		static void Flush();

		static void DrawModel(const MeshRendererComponent& model, const glm::mat4& transform);

	private:
		static void AddBatch(const UUID& material);
	private:
		static Renderer3DStorage s_Data;
	};
}