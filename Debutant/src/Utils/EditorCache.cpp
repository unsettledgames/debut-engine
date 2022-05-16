#include "EditorCache.h"

using namespace Debut;

namespace Debutant
{
	static AssetCache<Ref<Texture2D>> s_TextureCache(256, "Editor textures");

	AssetCache<Ref<Texture2D>>& EditorCache::Textures()
	{
		return s_TextureCache;
	}

}