#pragma once
#include <string>
#include <Debut/Core/Core.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/AssetManager/AssetCache.h>
#include <Debut/Rendering/Shader.h>

using namespace Debut;

namespace Debut
{
	class EditorCache
	{
	public:
		static AssetCache<std::string, Ref<Texture2D>>& Textures();
		static AssetCache<std::string, Ref<Shader>>& Shaders();
	};
}