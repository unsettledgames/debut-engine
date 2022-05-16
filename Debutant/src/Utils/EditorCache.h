#pragma once
#include <string>
#include <Debut/Core/Core.h>
#include <Debut/Renderer/Texture.h>
#include <Debut/Utils/AssetCache.h>

using namespace Debut;

// This horror will probably become the resources manager sooner or later (probably later)
namespace Debutant
{
	class EditorCache
	{
	public:
		static AssetCache<Ref<Texture2D>>& Textures();
	};
}