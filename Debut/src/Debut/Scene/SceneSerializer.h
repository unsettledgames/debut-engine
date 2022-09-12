#pragma once

#include <string>

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Debut
{
	class Scene;

	struct EntitySceneNode;

	class SceneSerializer
	{
	public:
		SceneSerializer(Ref<Scene> scene) : m_Scene(scene){}
		~SceneSerializer() {}

		void SerializeText(const std::string& fileName, EntitySceneNode& sceneGraph, const Ref<Scene> scene);
		void SerializeBin(const std::string& name) {};
		void SerializeEntity(EntitySceneNode& entity, YAML::Emitter& out);

		std::pair<Ref<Scene>, EntitySceneNode*> DeserializeText(const std::string& name);
		bool DeserializeBin(const std::string& name) { return false; };
		EntitySceneNode* DeserializeEntity(YAML::Node& node);

	private:
		Ref<Scene> m_Scene;
	};
}

