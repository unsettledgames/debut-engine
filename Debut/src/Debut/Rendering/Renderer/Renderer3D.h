#pragma once


namespace Debut
{
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;

	class Material;
	class Skybox;
	class Texture2D;
	class Camera;
	class Mesh;

	struct MeshRendererComponent;
	struct LightComponent;
	struct ShaderUniform;

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
		std::vector<LightComponent*> Lights;
		std::vector<ShaderUniform> GlobalUniforms;
	};

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(Camera& camera, Ref<Skybox> skybox, glm::mat4& transform, std::vector<LightComponent*>& lights,
			std::vector<ShaderUniform> globalUniforms);
		static void EndScene();
		static void Flush();

		static void DrawModel(const MeshRendererComponent& model, const glm::mat4& transform);
		static void DrawModel(Mesh& mesh, Material& material, const glm::mat4& transform, bool instanced = false);

		static void SendLights(Material& material);
		static void SendGlobals(Material& material);

	private:
		static void AddBatch(const UUID& material);
	private:
		static Renderer3DStorage s_Data;
	};
}