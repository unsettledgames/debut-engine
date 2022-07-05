#pragma once

#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Scene/Components.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Camera.h>

/*
	Ok, so, real talk. Batch rendering in 3D isn't that useful for general purpose.
		- One draw call per model IS reasonable, even if the geometry is different but the material is the same
		- Batch rendering in 3D is useful for instanced rendering. I've basically implemented half instanced rendering

		- Sooo, right now I want to focus on rendering as many models I want. So I'll have to get rid of the current "batch rendering"
		  system. Not getting rid of it entirely, just stop using them for general purpose: in the future I'd like to add some kind
		  of "instantiate" button for a model, so that instanced rendering can work for those models.

		- The shader binding optimization should still be applied though: when switching model, don't unbind the shader and save the
		  UUID of the last shader that was being used. Then, instead of rebinding it, just submit new uniforms before rendering.

	Renderer3D:
		- Have a VertexArray that is reserved to drawing one model at a time
		- When DrawModel is called, we check whether or not the model is instanced: if it is, we batch render it, otherwise we just draw
		  it normally.
*/

namespace Debut
{
	struct RenderBatch3D
	{
		std::unordered_map<std::string, Ref<VertexBuffer>> Buffers;
		std::vector<int> Indices;

		Ref<VertexArray> VertexArray;
		Ref<IndexBuffer> IndexBuffer;
		Ref<Material> Material;
	};

	struct Renderer3DStorage
	{
		uint32_t StartupBufferSize = 4096;
		uint32_t MaxTextures = 32;
		uint32_t MaxBatches = 64;
		uint32_t MaxMeshesPerBatch = 16384;
		uint32_t MaxVerticesPerBatch = 36000000;

		Ref<VertexArray> VertexArray;
		Ref<IndexBuffer> IndexBuffer;
		std::unordered_map<std::string, Ref<VertexBuffer>> VertexBuffers;

		std::vector<Ref<Texture2D>> Textures;
		// One batch per Material
		std::unordered_map<UUID, RenderBatch3D*> Batches;

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