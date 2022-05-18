#include <Debut/dbtpch.h>
#include <Debut/AssetManager/AssetManager.h>

namespace Debut
{
	static AssetCache<std::string, Ref<Texture2D>> s_TextureCache;

	void AssetManager::SubmitTexture(const std::string& path)
	{
		if (s_TextureCache.Has(path))
			return;

		Ref<Texture2D> toAdd = Texture2D::Create(path);
		s_TextureCache.Put(path, toAdd);
	}

	void AssetManager::SubmitTexture(Ref<Texture2D>& texture)
	{
		if (s_TextureCache.Has(texture->GetPath()))
			return;

		s_TextureCache.Put(texture->GetPath(), texture);
		return;
	}

	Ref<Texture2D> AssetManager::RequestTexture(const std::string& id)
	{
		if (s_TextureCache.Has(id))
			return s_TextureCache.Get(id);

		Ref<Texture2D> toAdd = Texture2D::Create(id);
		s_TextureCache.Put(id, toAdd);

		return toAdd;
	}
}