#include "EditorCache.h"

namespace Debut
{
	static AssetCache<std::string, Ref<Texture2D>> s_TextureCache(256, "Editor textures");

	AssetCache<std::string, Ref<Texture2D>>& EditorCache::Textures()
	{
		return s_TextureCache;
	}

}