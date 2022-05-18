#pragma once
#include <string>
#include <Debut/Core/Core.h>
#include <Debut/Renderer/Texture.h>
#include <Debut/AssetManager/AssetCache.h>

using namespace Debut;

// This horror will probably become the resources manager sooner or later (probably later)
namespace Debut
{
	class EditorCache
	{
	public:
		static AssetCache<std::string, Ref<Texture2D>>& Textures();
	};
}