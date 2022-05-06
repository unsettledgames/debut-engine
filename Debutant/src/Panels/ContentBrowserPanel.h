#pragma once

#include <Debut/Scene/Scene.h>
#include <Debut/Core/Core.h>
#include <Debut/Renderer/Texture.h>
#include <filesystem>
#include <unordered_map>

namespace Debut
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void SetContext(const Ref<Scene>& scene) { m_Context = scene; }

	private:
		Ref<Texture2D> GetFileIcon(const std::string& extension);

	private:
		Ref<Scene> m_Context;
		std::filesystem::path m_CurrDirectory;

		std::unordered_map<std::string, Ref<Texture2D>> m_Icons;
	};
}

