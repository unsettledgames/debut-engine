#pragma once

#include <Debut/Rendering/Structures/Frustum.h>

namespace Debut
{
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;
	class ShadowMap;

	class Material;
	class Skybox;
	class Texture2D;
	class SceneCamera;
	class Mesh;

	struct MeshRendererComponent;
	struct LightComponent;
	struct ShaderUniform;

	enum class RenderingPass { Shadow = 0, Shaded };

	struct RenderBatch3D
	{
		std::unordered_map<std::string, Ref<VertexBuffer>> Buffers;
		std::vector<int> Indices;

		Ref<VertexArray> VertexArray;
		Ref<IndexBuffer> IndexBuffer;
		Ref<Material> Material;
	};

	struct Renderer3DStats
	{
		uint32_t NShadowPasses = 0;
		uint32_t DrawCalls = 0;
		uint32_t Triangles = 0;
		uint32_t ShadowTriangles = 0;
		uint32_t ShadowDrawCalls = 0;
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

		// Camera data (might as well store the whole camera at this point)
		glm::mat4 CameraTransform;
		glm::mat4 CameraView;
		glm::mat4 CameraProjection;
		float CameraNear;
		float CameraFar;
		Frustum CameraFrustum;

		std::vector<LightComponent*> Lights;
		std::vector<ShaderUniform> GlobalUniforms;

		// Extra materials for special rendering modes
		Ref<Material> UntexturedMaterial;
		Ref<Material> VisualizeDepthmapMaterial;
		Ref<Material> DepthmapMaterial;

		// Shadow map
		std::vector<Ref<ShadowMap>> ShadowMaps;

		RenderingPass CurrentPass = RenderingPass::Shaded;
	};

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(SceneCamera& camera, Ref<Skybox> skybox, const glm::mat4& transform,
			std::vector<LightComponent*>& lights, std::vector<ShaderUniform>& globalUniforms, std::vector<Ref<ShadowMap>> shadowMaps);
		static void EndScene();
		static void Flush();

		static void BeginShadow(Ref<ShadowMap> shadowMap, SceneCamera& camera);
		static void EndShadow();

		static void DrawModel(const MeshRendererComponent& model, const glm::mat4& transform, int entityID);
		static void DrawModel(Mesh& mesh, Material& material, const glm::mat4& transform, int entityID, bool instanced = false);

		static void SendLights(Material& material);
		static void SendGlobals(Material& material);

		static inline Renderer3DStats GetStats() { return s_PrevStats; }
		static void ResetStats();

	private:
		static void AddBatch(const UUID& material);
	private:
		static Renderer3DStorage s_Data;
		static Renderer3DStats s_Stats;
		static Renderer3DStats s_PrevStats;
	};
}