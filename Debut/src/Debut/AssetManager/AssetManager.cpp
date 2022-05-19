#include <Debut/dbtpch.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Physics/PhysicsMaterial2D.h>

namespace Debut
{
	static AssetCache<std::string, Ref<Texture2D>> s_TextureCache;
	static AssetCache<std::string, Ref<PhysicsMaterial2D>> s_PhysicsMaterial2DCache;

	// ASSET SUBMISSION BY PATH

	template <typename T>
	void AssetManager::Submit(const std::string& path)
	{

	}

	template <>
	void AssetManager::Submit<Texture2D>(const std::string& path)
	{
		if (s_TextureCache.Has(path))
			return;

		Ref<Texture2D> toAdd = Texture2D::Create(path);
		s_TextureCache.Put(path, toAdd);
	}

	template<>
	void AssetManager::Submit<PhysicsMaterial2D>(const std::string& path)
	{
		if (s_PhysicsMaterial2DCache.Has(path))
			return;

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(path);
		s_PhysicsMaterial2DCache.Put(path, toAdd);
	}

	// ASSET SUBMISSION BY REFERENCE

	template <typename T>
	void AssetManager::Submit(const Ref<T>& toAdd)
	{

	}

	template <>
	void AssetManager::Submit<Texture2D>(const Ref<Texture2D>& texture)
	{
		if (s_TextureCache.Has(texture->GetPath()))
			return;

		s_TextureCache.Put(texture->GetPath(), texture);
		return;
	}

	template <>
	void AssetManager::Submit<PhysicsMaterial2D>(const Ref<PhysicsMaterial2D>& material)
	{
		if (s_PhysicsMaterial2DCache.Has(material->GetPath()))
			return;

		s_PhysicsMaterial2DCache.Put(material->GetPath(), material);
		return;
	}

	// ASSET REQUESTS

	template <typename T>
	Ref<T> AssetManager::Request(const std::string& id)
	{

	}

	template<>
	Ref<Texture2D> AssetManager::Request<Texture2D>(const std::string& id)
	{
		if (s_TextureCache.Has(id))
			return s_TextureCache.Get(id);

		Ref<Texture2D> toAdd;
		if (id == "")
			toAdd = Texture2D::Create(1, 1);
		else
			toAdd = Texture2D::Create(id);
		s_TextureCache.Put(id, toAdd);

		return toAdd;
	}

	template<>
	Ref<PhysicsMaterial2D> AssetManager::Request<PhysicsMaterial2D>(const std::string& id)
	{
		if (s_PhysicsMaterial2DCache.Has(id))
			return s_PhysicsMaterial2DCache.Get(id);

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(id);
		s_PhysicsMaterial2DCache.Put(id, toAdd);

		return toAdd;
	}
}