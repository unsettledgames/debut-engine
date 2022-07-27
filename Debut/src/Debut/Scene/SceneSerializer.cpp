#include "Debut/dbtpch.h"
#include "SceneSerializer.h"
#include "Components.h"
#include "Debut/Utils/CppUtils.h"
#include <Debut/Utils/YamlUtils.h>
#include <Debut/AssetManager/AssetManager.h>

namespace Debut
{
	static std::string Rb2DTypeToString(Rigidbody2DComponent::BodyType type)
	{
		switch (type)
		{
		case Rigidbody2DComponent::BodyType::Static: return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic: return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		default:
			DBT_CORE_ASSERT(false, "Unknown body type {0}", (int)type);
			return "";
		}
	}
	static Rigidbody2DComponent::BodyType StringToRb2DType(const std::string& type)
	{
		if (type == "Static") return Rigidbody2DComponent::BodyType::Static;
		if (type == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
		if (type == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;
		
		DBT_CORE_ASSERT(false, "Unknown body type {0}", type);
		return Rigidbody2DComponent::BodyType::Static;
	}
	

	template <typename T>
	static void SerializeComponent(Entity e, const std::string& name, YAML::Emitter& out)
	{
		if (!e.HasComponent<T>())
			return;

		out << YAML::Key << name << YAML::Value;
		out << YAML::BeginMap;
		SerializeComponent(e.GetComponent<T>(), out);
		out << YAML::EndMap;
	}

	static void SerializeComponent(const IDComponent& i, YAML::Emitter& out)
	{
		out << YAML::Key << "ID" << YAML::Value << i.ID;
	}

	static void SerializeComponent(const TagComponent& t, YAML::Emitter& out)
	{
		out << YAML::Key << "Name" << YAML::Value << t.Name;
		out << YAML::Key << "Tag" << YAML::Value << t.Tag;
	}

	static void SerializeComponent(TransformComponent& t, YAML::Emitter& out)
	{
		out << YAML::Key << "Translation" << YAML::Value << t.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << t.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << t.Scale;
		out << YAML::Key << "Parent" << YAML::Value << (!t.Parent ? 0 : (uint64_t)t.Parent.GetComponent<IDComponent>().ID);
	}

	static void SerializeComponent(const CameraComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "FixedAspectRatio" << YAML::Value << c.FixedAspectRatio;
		out << YAML::Key << "Primary" << YAML::Value << c.Primary;

		out << YAML::Key << "CameraData" << YAML::Value;
		out << YAML::BeginMap;

		out << YAML::Key << "ProjectionType" << YAML::Value << (int)c.Camera.GetProjectionType();
		out << YAML::Key << "OrthoNear" << YAML::Value << c.Camera.GetOrthoNearClip();
		out << YAML::Key << "OrthoFar" << YAML::Value << c.Camera.GetOrthoFarClip();
		out << YAML::Key << "PerspNear" << YAML::Value << c.Camera.GetPerspNearClip();
		out << YAML::Key << "PerspFar" << YAML::Value << c.Camera.GetPerspFarClip();
		out << YAML::Key << "OrthoSize" << YAML::Value << c.Camera.GetOrthoSize();
		out << YAML::Key << "PerspFOV" << YAML::Value << c.Camera.GetPerspFOV();

		out << YAML::EndMap;
	}

	static void SerializeComponent(const SpriteRendererComponent& s, YAML::Emitter& out)
	{
		out << YAML::Key << "Color" << YAML::Value << s.Color;
		out << YAML::Key << "Texture" << YAML::Value << s.Texture;
		out << YAML::Key << "TilingFactor" << YAML::Value << s.TilingFactor;
	}

	static void SerializeComponent(const MeshRendererComponent& s, YAML::Emitter& out)
	{
		out << YAML::Key << "Mesh" << YAML::Value << s.Mesh;
		out << YAML::Key << "Material" << YAML::Value << s.Material;
		out << YAML::Key << "Instanced" << YAML::Value << s.Instanced;
	}

	static void SerializeComponent(const Rigidbody2DComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "Type" << YAML::Value << Rb2DTypeToString(c.Type);
		out << YAML::Key << "FixedRotation" << YAML::Value << c.FixedRotation;
	}

	static void SerializeComponent(const BoxCollider2DComponent& c, YAML::Emitter& out)
	{
		Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(c.Material);

		out << YAML::Key << "Size" << YAML::Value << c.Size;
		out << YAML::Key << "Offset" << YAML::Value << c.Offset;

		out << YAML::Key << "Material";

		if (material != nullptr)
			out << YAML::Value << material->GetID();
		else
			out << YAML::Value << 0;
	}

	static void SerializeComponent(const CircleCollider2DComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "Radius" << YAML::Value << c.Radius;
		out << YAML::Key << "Offset" << YAML::Value << c.Offset;

		out << YAML::Key << "Density" << YAML::Value << c.Density;
		out << YAML::Key << "Friction" << YAML::Value << c.Friction;
		out << YAML::Key << "Restitution" << YAML::Value << c.Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << c.RestitutionThreshold;
	}

	template <typename T>
	static void DeserializeComponent(Entity e, YAML::Node& in, Ref<Scene> scene = nullptr)
	{
		DeserializeComponent<T>(e, in, scene);
	}

	template <>
	static void DeserializeComponent<IDComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in) return;
		IDComponent& id = e.GetComponent<IDComponent>();
		id.ID = in["ID"].as<uint64_t>();
	}

	template <>
	static void DeserializeComponent<TransformComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		TransformComponent& transform = e.GetComponent<TransformComponent>();

		transform.Translation = in["Translation"].as<glm::vec3>();
		transform.Rotation = in["Rotation"].as<glm::vec3>();
		transform.Scale = in["Scale"].as<glm::vec3>();
		transform.Parent = in["Parent"] ? scene->GetEntityByID(in["Parent"].as<uint64_t>()) : Entity(entt::null, nullptr);
	}

	template <>
	static void DeserializeComponent<CameraComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		CameraComponent& cc = e.AddComponent<CameraComponent>();

		cc.FixedAspectRatio = in["FixedAspectRatio"].as<bool>();
		cc.Primary = in["Primary"].as<bool>();

		cc.Camera.SetOrthoSize(in["CameraData"]["OrthoSize"].as<float>());
		cc.Camera.SetOrthoNearClip(in["CameraData"]["OrthoNear"].as<float>());
		cc.Camera.SetOrthoFarClip(in["CameraData"]["OrthoFar"].as<float>());

		cc.Camera.SetPerspFOV(in["CameraData"]["PerspFOV"].as<float>());
		cc.Camera.SetPerspNearClip(in["CameraData"]["PerspNear"].as<float>());
		cc.Camera.SetPerspFarClip(in["CameraData"]["PerspFar"].as<float>());

		cc.Camera.SetProjectionType((SceneCamera::ProjectionType)in["CameraData"]["ProjectionType"].as<int>());
	}

	template<>
	static void DeserializeComponent<SpriteRendererComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		SpriteRendererComponent& sc = e.AddComponent<SpriteRendererComponent>();
		sc.Color = in["Color"].as<glm::vec4>();
		if (in["Texture"]) sc.Texture = in["Texture"].as<uint64_t>();
		if (in["TilingFactor"])		sc.TilingFactor = in["TilingFactor"].as<float>();
	}

	template<>
	static void DeserializeComponent<MeshRendererComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		MeshRendererComponent& mr = e.AddComponent<MeshRendererComponent>();
		mr.Instanced = in["Instanced"].as<bool>();
		mr.Material = in["Material"].as<uint64_t>();
		mr.Mesh = in["Mesh"].as<uint64_t>();
	}

	template<>
	static void DeserializeComponent<Rigidbody2DComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		Rigidbody2DComponent& rb2d = e.AddComponent<Rigidbody2DComponent>();
		rb2d.FixedRotation = in["FixedRotation"].as<bool>();
		rb2d.Type = StringToRb2DType(in["Type"].as<std::string>());
	}

	template<>
	static void DeserializeComponent<BoxCollider2DComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		BoxCollider2DComponent& bc2d = e.AddComponent<BoxCollider2DComponent>();
		Ref<PhysicsMaterial2D> material = in["Material"] ? AssetManager::Request<PhysicsMaterial2D>(in["Material"].as<uint64_t>()) : nullptr;

		bc2d.Offset = in["Offset"].as<glm::vec2>();
		bc2d.Size = in["Size"].as<glm::vec2>();
		bc2d.Material = material ? material->GetID() : 0;
	}

	template<>
	static void DeserializeComponent<CircleCollider2DComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		CircleCollider2DComponent& bc2d = e.AddComponent<CircleCollider2DComponent>();

		bc2d.Density = in["Density"].as<float>();
		bc2d.Friction = in["Friction"].as<float>();
		bc2d.Restitution = in["Restitution"].as<float>();
		bc2d.RestitutionThreshold = in["RestitutionThreshold"].as<float>();

		bc2d.Offset = in["Offset"].as<glm::vec2>();
		bc2d.Radius = in["Radius"].as<float>();
	}

	void SceneSerializer::SerializeEntity(EntitySceneNode& node, YAML::Emitter& out)
	{
		Entity entity = node.EntityData;
		out << YAML::BeginMap << YAML::Key << "Entity: " << YAML::Value << entity.ID();

		SerializeComponent<IDComponent>(entity, "IDComponent", out);
		SerializeComponent<TagComponent>(entity, "TagComponent", out);
		SerializeComponent<TransformComponent>(entity, "TransformComponent", out);
		SerializeComponent<CameraComponent>(entity, "CameraComponent", out);
		SerializeComponent<SpriteRendererComponent>(entity, "SpriteRendererComponent", out);
		SerializeComponent<MeshRendererComponent>(entity, "MeshRendererComponent", out);
		SerializeComponent<Rigidbody2DComponent>(entity, "Rigidbody2DComponent", out);
		SerializeComponent<BoxCollider2DComponent>(entity, "BoxCollider2DComponent", out);
		SerializeComponent<CircleCollider2DComponent>(entity, "CircleCollider2DComponent", out);

		out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < node.Children.size(); i++)
			SerializeEntity(node.Children[i], out);
		out << YAML::EndSeq;

		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeText(const std::string& fileName)
	{
		YAML::Emitter out;
		std::ofstream outFile(fileName);
		
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled scene";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (uint32_t i = 0; i < m_Scene->m_CachedSceneGraph->Children.size(); i++)
			SerializeEntity(m_Scene->m_CachedSceneGraph->Children[i],out);

		out << YAML::EndSeq;
		out << YAML::EndMap;

		Log.CoreInfo("{0}", out.c_str());

		outFile << out.c_str();
	}

	bool SceneSerializer::DeserializeText(const std::string& fileName)
	{
		if (!CppUtils::String::endsWith(fileName, ".debut"))
			return false;

		std::ifstream inFile(fileName);
		std::stringstream strStream;

		strStream << inFile.rdbuf();

		YAML::Node in = YAML::Load(strStream.str());

		if (!in["Scene"])
			return false;

		auto entities = in["Entities"];

		if (entities)
		{
			for (auto yamlEntity : entities)
			{
				DeserializeEntity(yamlEntity);
			}
		}

		return true;
	}

	void SceneSerializer::DeserializeEntity(YAML::Node& yamlEntity)
	{
		// Create a new entity, set the tag and name
		auto tc = yamlEntity["TagComponent"];
		Entity entity = m_Scene->CreateEntity({}, tc["Name"].as<std::string>());
		entity.GetComponent<TagComponent>().Tag = tc["Tag"].as<std::string>();

		// Deserialize the other components
		DeserializeComponent<TransformComponent>(entity, yamlEntity["TransformComponent"], m_Scene);
		DeserializeComponent<CameraComponent>(entity, yamlEntity["CameraComponent"]);
		DeserializeComponent<SpriteRendererComponent>(entity, yamlEntity["SpriteRendererComponent"]);
		DeserializeComponent<MeshRendererComponent>(entity, yamlEntity["MeshRendererComponent"]);
		DeserializeComponent<Rigidbody2DComponent>(entity, yamlEntity["Rigidbody2DComponent"]);
		DeserializeComponent<BoxCollider2DComponent>(entity, yamlEntity["BoxCollider2DComponent"]);
		DeserializeComponent<CircleCollider2DComponent>(entity, yamlEntity["CircleCollider2DComponent"]);
		DeserializeComponent<IDComponent>(entity, yamlEntity["IDComponent"]);

		auto children = yamlEntity["Children"];
		for (auto child : children)
			DeserializeEntity(child);
	}
}