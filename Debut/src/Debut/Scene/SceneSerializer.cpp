#include "Debut/dbtpch.h"
#include <yaml-cpp/yaml.h>
#include "SceneSerializer.h"
#include "Components.h"

namespace Debut
{
	template <typename T>
	static void SerializeComponent(T component, const std::string& name, YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Component" << YAML::Value << name;

	}

	void SceneSerializer::SerializeText(const std::string& fileName)
	{
		YAML::Emitter out;
		
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled scene";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto entityId) 
		{
			Entity entity(entityId, m_Scene.get());
			if (entity.HasComponent<SpriteRendererComponent>())
			{
				SerializeComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>(), "SpriteRendererComponent", out);
			}
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		// foreach entity : scene
			// foreach component : entity
				// serialize the component
	}
}