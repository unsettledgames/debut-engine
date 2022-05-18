#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/Renderer/Texture.h>
#include <Debut/AssetManager/Asset.h>
#include <Debut/AssetManager/AssetCache.h>
#include <Debut/Physics/PhysicsMaterial2D.h>

namespace Debut
{
	// TODO: maintain a file containing the UUID -> path associations for assets
	// After this one is done I could probably add some automatic asset import!
	class AssetManager
	{
	public:
		AssetManager() = default;

		template<typename T>
		static void CreateAsset(const std::string& path)
		{
			std::string tmpPath = path;
			std::ifstream test(tmpPath);
			while (test.good())
			{
				// Append "-copy" before the extension to avoid overwriting stuff
				tmpPath = tmpPath.substr(0, tmpPath.find_last_of(".")) + "-copy" + tmpPath.substr(tmpPath.find_last_of("."), tmpPath.length() - tmpPath.find_last_of("."));
				test = std::ifstream(tmpPath);
			}
			
			// Create the empty file
			T::SaveDefaultConfig(tmpPath);
		}

		static void SubmitTexture(const std::string& path);
		static void SubmitTexture(Ref<Texture2D>& texture);

		static void SubmitPhysicsMaterial2D(const std::string& path);
		static void SubmitPhysicsMaterial2D(Ref<PhysicsMaterial2D>& texture);

		static Ref<Texture2D> RequestTexture(const std::string& id);
		static Ref<PhysicsMaterial2D> RequestPhysicsMaterial2D(const std::string& id);
	};
}
