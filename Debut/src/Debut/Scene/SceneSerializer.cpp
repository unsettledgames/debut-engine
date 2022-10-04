#include "Debut/dbtpch.h"
#include "SceneSerializer.h"
#include "Components.h"
#include "Debut/Utils/CppUtils.h"
#include <Debut/Utils/YamlUtils.h>
#include <Debut/Scene/Scene.h>
#include <Debut/Rendering/Resources/Skybox.h>
#include <yaml-cpp/yaml.h>

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
		out << YAML::Key << "OrthoNear" << YAML::Value << c.Camera.GetNearPlane();
		out << YAML::Key << "OrthoFar" << YAML::Value << c.Camera.GetFarPlane();
		out << YAML::Key << "PerspNear" << YAML::Value << c.Camera.GetNearPlane();
		out << YAML::Key << "PerspFar" << YAML::Value << c.Camera.GetFarPlane();
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
		out << YAML::Key << "Size" << YAML::Value << c.Size;
		out << YAML::Key << "Offset" << YAML::Value << c.Offset;

		out << YAML::Key << "Material"  << YAML::Value << c.Material;
	}

	static void SerializeComponent(const CircleCollider2DComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "Radius" << YAML::Value << c.Radius;
		out << YAML::Key << "Offset" << YAML::Value << c.Offset;

		out << YAML::Key << "Material" << YAML::Value << c.Material;
	}

	static void SerializeComponent(const PolygonCollider2DComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "Offset" << YAML::Value << c.Offset;
		out << YAML::Key << "Points" << YAML::Value << YAML::BeginSeq;

		for (auto& point : c.Points)
			out << point;

		out << YAML::EndSeq;
		out << YAML::Key << "Material" << YAML::Value << c.Material;
	}

	static void SerializeComponent(const DirectionalLightComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "Direction" << YAML::Value << c.Direction;
		out << YAML::Key << "Color" << YAML::Value << c.Color;
		out << YAML::Key << "Intensity" << YAML::Value << c.Intensity;
	}

	static void SerializeComponent(const PointLightComponent& c, YAML::Emitter& out)
	{
		out << YAML::Key << "Color" << YAML::Value << c.Color;
		out << YAML::Key << "Intensity" << YAML::Value << c.Intensity;
		out << YAML::Key << "Radius" << YAML::Value << c.Radius;
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
		cc.Camera.SetNearPlane(in["CameraData"]["OrthoNear"].as<float>());
		cc.Camera.SetFarPlane(in["CameraData"]["OrthoFar"].as<float>());

		cc.Camera.SetPerspFOV(in["CameraData"]["PerspFOV"].as<float>());
		cc.Camera.SetNearPlane(in["CameraData"]["PerspNear"].as<float>());
		cc.Camera.SetFarPlane(in["CameraData"]["PerspFar"].as<float>());

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

		bc2d.Offset = in["Offset"].as<glm::vec2>();
		bc2d.Size = in["Size"].as<glm::vec2>();
		bc2d.Material = in["Material"] ? in["Material"].as<uint64_t>() : 0;
	}

	template<>
	static void DeserializeComponent<CircleCollider2DComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		CircleCollider2DComponent& bc2d = e.AddComponent<CircleCollider2DComponent>();

		bc2d.Offset = in["Offset"].as<glm::vec2>();
		bc2d.Radius = in["Radius"].as<float>();
		bc2d.Material = in["Material"].as<uint64_t>();
	}

	template<>
	static void DeserializeComponent<PolygonCollider2DComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		PolygonCollider2DComponent& pc2d = e.AddComponent<PolygonCollider2DComponent>();

		pc2d.Offset = in["Offset"].as<glm::vec2>();
		uint32_t i = 0;
		// Remove default points
		for (uint32_t j = 0; j < 4; j++)
			pc2d.RemovePoint(0);

		for (auto& vec : in["Points"])
		{
			pc2d.AddPoint();
			pc2d.SetPoint(i, vec.as<glm::vec2>());
			i++;
		}
		pc2d.Triangulate();
		pc2d.Material = in["Material"].as<uint64_t>();
	}

	template<>
	static void DeserializeComponent<DirectionalLightComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		DirectionalLightComponent& dl = e.AddComponent<DirectionalLightComponent>();

		dl.Direction = in["Direction"].as<glm::vec3>();
		dl.Color = in["Color"].as<glm::vec3>();
		dl.Intensity = in["Intensity"].as<float>();
	}

	template<>
	static void DeserializeComponent<PointLightComponent>(Entity e, YAML::Node& in, Ref<Scene> scene)
	{
		if (!in)
			return;
		PointLightComponent& dl = e.AddComponent<PointLightComponent>();

		dl.Color = in["Color"].as<glm::vec3>();
		dl.Intensity = in["Intensity"].as<float>();
		dl.Radius = in["Radius"].as<float>();
	}

	void SceneSerializer::SerializeEntity(EntitySceneNode& node, YAML::Emitter& out)
	{
		Entity entity = node.EntityData;
		out << YAML::BeginMap << YAML::Key << "Entity: " << YAML::Value << entity.ID();
		out << YAML::Key << "HierarchyOrder" << YAML::Value << node.IndexInNode;

		SerializeComponent<IDComponent>(entity, "IDComponent", out);
		SerializeComponent<TagComponent>(entity, "TagComponent", out);
		SerializeComponent<TransformComponent>(entity, "TransformComponent", out);
		SerializeComponent<CameraComponent>(entity, "CameraComponent", out);
		SerializeComponent<SpriteRendererComponent>(entity, "SpriteRendererComponent", out);
		SerializeComponent<MeshRendererComponent>(entity, "MeshRendererComponent", out);
		SerializeComponent<Rigidbody2DComponent>(entity, "Rigidbody2DComponent", out);
		SerializeComponent<BoxCollider2DComponent>(entity, "BoxCollider2DComponent", out);
		SerializeComponent<CircleCollider2DComponent>(entity, "CircleCollider2DComponent", out);
		SerializeComponent<PolygonCollider2DComponent>(entity, "PolygonCollider2DComponent", out);
		SerializeComponent<DirectionalLightComponent>(entity, "DirectionalLightComponent", out);
		SerializeComponent<PointLightComponent>(entity, "PointLightComponent", out);

		out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < node.Children.size(); i++)
			SerializeEntity(*node.Children[i], out);
		out << YAML::EndSeq;

		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeText(const std::string& fileName, EntitySceneNode& sceneGraph)
	{
		YAML::Emitter out;
		std::ofstream outFile(fileName);
		
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled scene";
		
		out << YAML::Key << "Lighting" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "AmbientLightColor" << YAML::Value << m_Scene->GetAmbientLight();
		out << YAML::Key << "AmbientLightIntensity" << YAML::Value << m_Scene->GetAmbientLightIntensity();
		Ref<Skybox> skybox = m_Scene->GetSkybox();
		out << YAML::Key << "Skybox" << YAML::Value << (skybox == nullptr ? 0 : m_Scene->GetSkybox()->GetID());
		out << YAML::EndMap;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (uint32_t i = 0; i < sceneGraph.Children.size(); i++)
			SerializeEntity(*sceneGraph.Children[i], out);

		out << YAML::EndSeq;
		out << YAML::EndMap;

		Log.CoreInfo("{0}", out.c_str());

		outFile << out.c_str();
	}

	EntitySceneNode* SceneSerializer::DeserializeText(const std::string& fileName)
	{
		if (!CppUtils::String::EndsWith(fileName, ".debut"))
			return nullptr;

		std::ifstream inFile(fileName);
		std::stringstream strStream;

		strStream << inFile.rdbuf();

		YAML::Node in = YAML::Load(strStream.str());

		if (!in["Scene"])
			return nullptr;

		auto entities = in["Entities"];
		EntitySceneNode* sceneTree = new EntitySceneNode();
		sceneTree->IsRoot = true;

		if (entities)
			for (auto yamlEntity : entities)
				sceneTree->Children.push_back(DeserializeEntity(yamlEntity));

		YAML::Node lighting = in["Lighting"];
		if (lighting["AmbientLightColor"].IsDefined())
			m_Scene->SetAmbientLight(lighting["AmbientLightColor"].as<glm::vec3>());
		if (lighting["AmbientLightIntensity"].IsDefined())
			m_Scene->SetAmbientLightIntensity(lighting["AmbientLightIntensity"].as<float>());
		if (lighting["Skybox"].IsDefined() && lighting["Skybox"].as<uint64_t>() != 0)
			m_Scene->SetSkybox(lighting["Skybox"].as<uint64_t>());

		return sceneTree;
	}

	EntitySceneNode* SceneSerializer::DeserializeEntity(YAML::Node& yamlEntity)
	{
		// Create a new entity, set the tag and name
		auto tc = yamlEntity["TagComponent"];
		Entity entity = m_Scene->CreateEntity({}, tc["Name"].as<std::string>());
		EntitySceneNode* node = new EntitySceneNode(false, entity);
		node->IndexInNode = yamlEntity["HierarchyOrder"].as<uint32_t>();
		entity.GetComponent<TagComponent>().Tag = tc["Tag"].as<std::string>();

		// Deserialize the other components
		DeserializeComponent<TransformComponent>(entity, yamlEntity["TransformComponent"], m_Scene);
		DeserializeComponent<CameraComponent>(entity, yamlEntity["CameraComponent"]);
		DeserializeComponent<SpriteRendererComponent>(entity, yamlEntity["SpriteRendererComponent"]);
		DeserializeComponent<MeshRendererComponent>(entity, yamlEntity["MeshRendererComponent"]);
		DeserializeComponent<Rigidbody2DComponent>(entity, yamlEntity["Rigidbody2DComponent"]);
		DeserializeComponent<BoxCollider2DComponent>(entity, yamlEntity["BoxCollider2DComponent"]);
		DeserializeComponent<PolygonCollider2DComponent>(entity, yamlEntity["PolygonCollider2DComponent"]);
		DeserializeComponent<CircleCollider2DComponent>(entity, yamlEntity["CircleCollider2DComponent"]);
		DeserializeComponent<IDComponent>(entity, yamlEntity["IDComponent"]);
		DeserializeComponent<DirectionalLightComponent>(entity, yamlEntity["DirectionalLightComponent"]);
		DeserializeComponent<PointLightComponent>(entity, yamlEntity["PointLightComponent"]);

		auto children = yamlEntity["Children"];
		for (auto child : children)
			node->Children.push_back(DeserializeEntity(child));

		return node;
	}
}