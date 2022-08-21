#include "EditorCache.h"

namespace Debut
{
	static AssetCache<std::string, Ref<Texture2D>> s_TextureCache(256, "Editor textures");
	static AssetCache<std::string, Ref<Shader>> s_ShaderCache(128, "Editor shaders");

	AssetCache<std::string, Ref<Texture2D>>& EditorCache::Textures()
	{
		return s_TextureCache;
	}

	AssetCache<std::string, Ref<Shader>>& EditorCache::Shaders()
	{
		return s_ShaderCache;
	}

}