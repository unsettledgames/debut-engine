#pragma once

#include "Scene.h"

namespace Debut
{
	class SceneSerializer
	{
	public:
		SceneSerializer(Ref<Scene> scene) : m_Scene(scene){}
		~SceneSerializer() {}

		void SerializeText(const std::string& name);
		void SerializeBin(const std::string& name);

		bool DeserializeText();
		bool DeserializeBin();

	private:
		Ref<Scene> m_Scene;
	};
}

