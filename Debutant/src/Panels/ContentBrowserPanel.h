#pragma once

#include <string>
#include <unordered_map>

#include <Debut/Scene/Scene.h>
#include <Debut/Core/Core.h>
#include <Debut/Renderer/Texture.h>
#include <filesystem>

using namespace Debut;

namespace Debutant
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void SetContext(const Ref<Scene>& scene) { m_Context = scene; }

	private:
		Ref<Texture2D> GetFileIcon(const std::filesystem::path& path);

	private:
		Ref<Scene> m_Context;
		std::filesystem::path m_CurrDirectory;

		std::unordered_map<std::string, std::string> m_Icons;
	};
}

