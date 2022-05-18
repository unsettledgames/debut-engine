#include <Debut/dbtpch.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Physics/PhysicsMaterial2D.h>

namespace Debut
{
	static AssetCache<std::string, Ref<Texture2D>> s_TextureCache;
	static AssetCache<std::string, Ref<PhysicsMaterial2D>> s_PhysicsMaterial2DCache;

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

	void AssetManager::SubmitPhysicsMaterial2D(const std::string& path)
	{
		if (s_PhysicsMaterial2DCache.Has(path))
			return;

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(path);
		s_PhysicsMaterial2DCache.Put(path, toAdd);
	}

	void AssetManager::SubmitPhysicsMaterial2D(Ref<PhysicsMaterial2D>& material)
	{
		if (s_PhysicsMaterial2DCache.Has(material->GetPath()))
			return;

		s_PhysicsMaterial2DCache.Put(material->GetPath(), material);
		return;
	}

	Ref<PhysicsMaterial2D> AssetManager::RequestPhysicsMaterial2D(const std::string& id)
	{
		if (s_PhysicsMaterial2DCache.Has(id))
			return s_PhysicsMaterial2DCache.Get(id);

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(id);
		s_PhysicsMaterial2DCache.Put(id, toAdd);

		return toAdd;
	}
}