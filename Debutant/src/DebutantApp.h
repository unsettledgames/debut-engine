#pragma once

#include <Debut/Core/Application.h>
#include <Scene/SceneManager.h>

namespace Debut
{
	class DebutantApp : public Application
	{
	public:
		DebutantApp();
		~DebutantApp() = default;

		static DebutantApp& Get() { return *((DebutantApp*)s_Instance); }
		SceneManager& GetSceneManager() { return m_SceneManager; }

	private:
		SceneManager m_SceneManager;
	};
}