#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Shader.h>

#include <unordered_map>
#include <string>
#include <stack>

namespace Debut
{
	struct PostProcessingVolume
	{
		std::string Name;
		UUID Shader;
		std::unordered_map<std::string, ShaderUniform> Properties;

		PostProcessingVolume() = default;
		PostProcessingVolume(UUID shader) : Shader(shader) {}
	};

	struct PostProcessingStackConfig
	{
		UUID ID;
		std::vector<PostProcessingVolume> Volumes;
		PostProcessingStackConfig() = default;
	};

	class PostProcessingStack
	{
	public:
		PostProcessingStack() = default;
		PostProcessingStack(const std::string& path, const std::string& metaFile = "");
		PostProcessingStack(const std::string& path, const PostProcessingStackConfig& config);

		static void SaveDefaultConfig(const std::string& path);
		static void SaveSettings(const std::string& path, const PostProcessingStackConfig& config);
		static PostProcessingStackConfig GetConfig(const std::string& path);

		inline void PushVolume(const PostProcessingVolume& volume) { m_Volumes.push_back(volume); }
		inline void PopVolume(const PostProcessingVolume& volume) { m_Volumes.pop_back(); }
		inline void RemoveVolume(uint32_t idx)  { m_Volumes.erase(m_Volumes.begin() + idx); }

		inline std::string GetPath() { return m_Path; }
		inline UUID GetID() { return m_ID; }

		void SetConfig(const PostProcessingStackConfig& config);

	private:
		UUID m_ID;
		std::string m_Path;
		std::vector<PostProcessingVolume> m_Volumes;
	};
}