#pragma once

#include "Scene.h"
#include <Debut/Events/KeyEvent.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	class Emitter;
}

namespace Debut
{
	class SceneSerializer
	{
	public:
		SceneSerializer(Ref<Scene> scene) : m_Scene(scene){}
		~SceneSerializer() {}

		void SerializeText(const std::string& fileName, EntitySceneNode& sceneGraph);
		void SerializeBin(const std::string& name) {};
		void SerializeEntity(EntitySceneNode& entity, YAML::Emitter& out);

		bool DeserializeText(const std::string& name);
		bool DeserializeBin(const std::string& name) { return false; };
		void DeserializeEntity(YAML::Node& node);

	private:
		Ref<Scene> m_Scene;
	};
}

