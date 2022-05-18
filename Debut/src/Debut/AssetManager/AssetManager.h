#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/Renderer/Texture.h>
#include <Debut/AssetManager/Asset.h>
#include <Debut/AssetManager/AssetCache.h>

namespace Debut
{
	class AssetManager
	{
	public:
		AssetManager() = default;

		static void SubmitTexture(const std::string& path);
		static void SubmitTexture(Ref<Texture2D>& texture);

		static Ref<Texture2D> RequestTexture(const std::string& id);
	};
}
