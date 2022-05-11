#include "Debut/dbtpch.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include "Debut/Scene/Entity.h"
#include "Debut/Scene/Components.h"
#include "Debut/Renderer/Renderer2D.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_fixture.h"

namespace Debut
{
	static b2BodyType DbtToBox2DBodyType(Rigidbody2DComponent::BodyType dbtType)
	{
		switch (dbtType)
		{
		case Rigidbody2DComponent::BodyType::Dynamic:	return b2BodyType::b2_dynamicBody;
		case Rigidbody2DComponent::BodyType::Kinematic:	return b2BodyType::b2_kinematicBody;
		case Rigidbody2DComponent::BodyType::Static:	return b2BodyType::b2_staticBody;
		default:
			DBT_CORE_ASSERT("The specified body type ({0}) is not supported", (int)dbtType);
			return b2BodyType::b2_staticBody;
		}
	}
	
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{

	}

	template<typename T>
	void Scene::OnComponentAdded(T& component, Entity entity)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded(TransformComponent& component, Entity entity) {}
	template<>
	void Scene::OnComponentAdded(SpriteRendererComponent& sr, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(TagComponent& tc, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(NativeScriptComponent& nsc, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(Rigidbody2DComponent& rb2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(BoxCollider2DComponent& bc2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(IDComponent& bc2d, Entity entity) { }

	template <typename Component>
	static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity> enttMap)
	{
		auto view = src.view<Component>();
		auto idView = dst.view<IDComponent>();

		for (auto e : view)
		{
			UUID id = src.get<IDComponent>(e).ID;
			entt::entity dstEnttID = enttMap.at(id);

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
	}

	template <typename Component>
	static void CopyComponentIfExists(Entity& dst, Entity& src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

	//TODO: OnComponentRemove, delete bodies

	template<>
	void Scene::OnComponentAdded(CameraComponent& camera, Entity entity)
	{
		camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto e : view)
		{
			const auto& camera = view.get<CameraComponent>(e);
			if (camera.Primary)
				return Entity{ e, this };
		}
		return {};
	}

	void Scene::OnEditorUpdate(Timestep ts, EditorCamera& camera)
	{
		DBT_PROFILE_SCOPE("Editor update");
		Renderer2D::BeginScene(camera);

		auto group = m_Registry.group<TransformComponent, SpriteRendererComponent>();
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		}

		Renderer2D::EndScene();
	}
	

	void Scene::OnRuntimeUpdate(Timestep ts)
	{
		// Update scripts
		{
			DBT_PROFILE_SCOPE("Scene: Script Update");
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = { entity, this };
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(ts);
				});
		}

		// Update physics
		{
			DBT_PROFILE_SCOPE("Scene: Physics2D Update");
			// TODO: physics settings
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_PhysicsWorld2D->Step(ts, velocityIterations, positionIterations);

			// Stuff moved, update the transforms to reflect that
			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				// TODO: cache bodies so that it's quicker to do this
				b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;
				const auto& position = body->GetPosition();
				auto& transform = entity.Transform();

				transform.Translation = { position.x, position.y, 0 };
				transform.Rotation.z = body->GetAngle();
			}

		}

		// Render sprites

		// Find the main camera of the scene
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				if (camera.Primary)
				{
					mainCamera = &(camera.Camera);
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			DBT_PROFILE_SCOPE("Renderer2D update");
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			auto group = m_Registry.group<TransformComponent, SpriteRendererComponent>();
			for (auto entity : group)
			{
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);

			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnRuntimeStart()
	{
		// SETUP PHYSICS!
		// TODO: physics settings
		m_PhysicsWorld2D = new b2World({b2Vec2(0.0f, -9.8f)});
		auto rigidbodyView = m_Registry.view<Rigidbody2DComponent>();
		auto boxView = m_Registry.view<BoxCollider2DComponent>();

		// Create Rigidbodies
		for (auto e : rigidbodyView)
		{
			Entity entity = { e, this };
			auto& transform = entity.Transform();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = DbtToBox2DBodyType(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y
			);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld2D->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			// Setup box collders
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(transform.Scale.x * bc2d.Size.x / 2, transform.Scale.y * bc2d.Size.y / 2);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		delete m_PhysicsWorld2D;
		m_PhysicsWorld2D = nullptr;
	}

	void Scene::DuplicateEntity(Entity& entity)
	{
		if (!entity)
			return;

		Entity duplicate = CreateEntity(entity.GetComponent<TagComponent>().Name + " Copy");
		
		CopyComponentIfExists<TransformComponent>(duplicate, entity);
		CopyComponentIfExists<SpriteRendererComponent>(duplicate, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(duplicate, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(duplicate, entity);
		CopyComponentIfExists<CameraComponent>(duplicate, entity);
		CopyComponentIfExists<NativeScriptComponent>(duplicate, entity);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };

		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);
		ret.AddComponent<IDComponent>();

		return ret;
	}

	Entity Scene::CreateEntity(const UUID& id, const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };

		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);
		IDComponent idC = ret.AddComponent<IDComponent>();
		idC.ID = id;

		return ret;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportHeight = other->m_ViewportHeight;
		newScene->m_ViewportWidth = other->m_ViewportWidth;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		auto idView = srcSceneRegistry.view<IDComponent>();

		// Copy old entities
		for (auto e : idView)
		{
			UUID id = srcSceneRegistry.get<IDComponent>(e).ID;
			auto& name = srcSceneRegistry.get<TagComponent>(e).Name;

			Entity newEntity = newScene->CreateEntity(id, name);
			enttMap[id] = newEntity;
		}

		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto& entity : view)
		{
			CameraComponent& camera = view.get<CameraComponent>(entity);
			if (!camera.FixedAspectRatio)
				camera.Camera.SetViewportSize(width, height);
		}

		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}
}