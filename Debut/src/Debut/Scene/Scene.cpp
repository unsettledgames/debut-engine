#include "Debut/dbtpch.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include "Debut/Scene/Entity.h"
#include "Debut/Scene/Components.h"
#include "Debut/Rendering/Shader.h"
#include "Debut/Rendering/Renderer/Renderer2D.h"
#include "Debut/Rendering/Renderer/Renderer3D.h"
#include "Debut/AssetManager/AssetManager.h"
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Rendering/Resources/Skybox.h>

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
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
	void Scene::OnComponentAdded(CircleCollider2DComponent& bc2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(IDComponent& bc2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(MeshRendererComponent& bc2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(DirectionalLightComponent& dl, Entity entity) {}
	template<>
	void Scene::OnComponentAdded(PointLightComponent& dl, Entity entity) {}

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

	void Scene::OnEditorUpdate(Timestep ts, Camera& camera)
	{
		DBT_PROFILE_SCOPE("Editor update");
		
		glm::mat4 transform = glm::inverse(camera.GetView());
		std::vector<ShaderUniform> globalUniforms = GetGlobalUniforms(transform[3]);
		std::vector<LightComponent*> lights;
		// Directional light
		auto lightGroup = m_Registry.view<TransformComponent, DirectionalLightComponent>();
		bool full = false;
		for (auto entity : lightGroup)
		{
			auto& [transform, light] = lightGroup.get<TransformComponent, DirectionalLightComponent>(entity);
			lights.push_back(&light);
			full = true;
		}
		DirectionalLightComponent tmpDirLight;
		if (!full)
		{
			tmpDirLight.Intensity = 0;
			lights.push_back(&tmpDirLight);
		}

		// Point lights
		auto pointLights = m_Registry.view<TransformComponent, PointLightComponent>();
		for (auto entity : pointLights)
		{
			auto& [transform, light] = pointLights.get<TransformComponent, PointLightComponent>(entity);
			// Update light position
			light.Position = transform.Translation;
			lights.push_back(&light);
		}

		// 3D Rendering
		Renderer3D::BeginScene(camera, m_Skybox, glm::inverse(camera.GetView()), lights, globalUniforms);
		
		auto group3D = m_Registry.view<TransformComponent, MeshRendererComponent>();
		for (auto entity : group3D)
		{
			auto& [transform, mesh] = group3D.get<TransformComponent, MeshRendererComponent>(entity);
			Renderer3D::DrawModel(mesh, transform.GetTransform());
		}

		Renderer3D::EndScene();

		// 2D Rendering
		Renderer2D::BeginScene(camera, glm::inverse(camera.GetView()));

		auto group = m_Registry.group<TransformComponent, SpriteRendererComponent>();
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);

			// Render debug info
			// Visualize box colliders
			auto boxView = m_Registry.view<BoxCollider2DComponent>();
			for (auto e : boxView)
			{
				Entity entity = { e, this };
				BoxCollider2DComponent component = m_Registry.get<BoxCollider2DComponent>(e);
				Renderer2D::DrawRect(entity.Transform().GetTransform(), component.Size, component.Offset, glm::vec4(0.2f, 1.0f, 0.4f, 1.0f));
			}
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
			// 3D Rendering
			{
				DBT_PROFILE_SCOPE("Renderer3D update");
				// Get lights
				glm::mat4 cameraView = mainCamera->GetView();
				std::vector<LightComponent*> lights;
				std::vector<ShaderUniform> globals = GetGlobalUniforms({ cameraView[0][3], cameraView[1][3], cameraView[2][3]});

				auto lightGroup = m_Registry.view<TransformComponent, DirectionalLightComponent>();
				for (auto entity : lightGroup)
				{
					auto& [transform, light] = lightGroup.get<TransformComponent, DirectionalLightComponent>(entity);
					lights.push_back(&light);
				}
				auto pointLights = m_Registry.view<TransformComponent, PointLightComponent>();
				for (auto entity : pointLights)
				{
					auto& [transform, light] = pointLights.get<TransformComponent, PointLightComponent>(entity);
					lights.push_back(&light);
				}
				Renderer3D::BeginScene(*mainCamera, m_Skybox, cameraTransform, lights, globals);

				auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
				for (auto entity : group)
				{
					auto& [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
					Renderer3D::DrawModel(mesh, transform.GetTransform());
				}

				Renderer3D::EndScene();
			}

			// 2D Rendering
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
	}

	void Scene::OnRuntimeStart()
	{
		// SETUP PHYSICS!
		// TODO: physics settings
		m_PhysicsWorld2D = new b2World({b2Vec2(0.0f, -9.8f)});
		auto rigidbodyView = m_Registry.view<Rigidbody2DComponent>();
		auto boxView = m_Registry.view<BoxCollider2DComponent>();
		auto circleView = m_Registry.view<CircleCollider2DComponent>();

		// Create Rigidbodies
		for (auto e : rigidbodyView)
		{
			Entity entity = { e, this };
			auto& transform = entity.Transform();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = DbtToBox2DBodyType(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld2D->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			// Setup box collders
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(bc2d.Material);

				b2PolygonShape boxShape;
				boxShape.SetAsBox(transform.Scale.x * bc2d.Size.x / 2, transform.Scale.y * bc2d.Size.y / 2, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;

				if (material != nullptr)
				{
					fixtureDef.density = material->GetDensity();
					fixtureDef.friction = material->GetFriction();
					fixtureDef.restitution = material->GetRestitution();
					fixtureDef.restitutionThreshold = material->GetRestitutionThreshold();
				}
				else
				{
					fixtureDef.density = PhysicsMaterial2D::DefaultSettings.Density;
					fixtureDef.friction = PhysicsMaterial2D::DefaultSettings.Friction;
					fixtureDef.restitution = PhysicsMaterial2D::DefaultSettings.Restitution;
					fixtureDef.restitutionThreshold = PhysicsMaterial2D::DefaultSettings.RestitutionThreshold;
				}
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
				Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(cc2d.Material);

				b2CircleShape circleShape;
				circleShape.m_radius = cc2d.Radius;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;

				if (material != nullptr)
				{
					fixtureDef.density = material->GetDensity();
					fixtureDef.friction = material->GetFriction();
					fixtureDef.restitution = material->GetRestitution();
					fixtureDef.restitutionThreshold = material->GetRestitutionThreshold();
				}
				else
				{
					fixtureDef.density = PhysicsMaterial2D::DefaultSettings.Density;
					fixtureDef.friction = PhysicsMaterial2D::DefaultSettings.Friction;
					fixtureDef.restitution = PhysicsMaterial2D::DefaultSettings.Restitution;
					fixtureDef.restitutionThreshold = PhysicsMaterial2D::DefaultSettings.RestitutionThreshold;
				}
				
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

		Entity duplicate = CreateEntity({}, entity.GetComponent<TagComponent>().Name + " Copy");
		
		CopyComponentIfExists<TransformComponent>(duplicate, entity);
		CopyComponentIfExists<SpriteRendererComponent>(duplicate, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(duplicate, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(duplicate, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(duplicate, entity);
		CopyComponentIfExists<CameraComponent>(duplicate, entity);
		CopyComponentIfExists<NativeScriptComponent>(duplicate, entity);
		CopyComponentIfExists<MeshRendererComponent>(duplicate, entity);
		CopyComponentIfExists<DirectionalLightComponent>(duplicate, entity);
		CopyComponentIfExists<PointLightComponent>(duplicate, entity);
		
		duplicate.Transform().Parent = duplicate.Transform().Parent;
	}

	Entity Scene::CreateEntity(Entity parent, const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };

		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);
		ret.AddComponent<IDComponent>();

		ret.Transform().Parent = parent;

		return ret;
	}

	Entity Scene::CreateEntity(Entity parent, const UUID& id, const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };

		IDComponent& idC = ret.AddComponent<IDComponent>();
		idC.ID = id;
		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);

		ret.Transform().Parent = parent;

		return ret;
	}

	Entity Scene::GetEntityByID(uint64_t id)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto& entity : view)
		{
			IDComponent& idComp = view.get<IDComponent>(entity);
			if (idComp.ID == id)
				return { entity, this };
		}

		return {};
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::SetSkybox(UUID skybox)
	{
		m_Skybox = AssetManager::Request<Skybox>(skybox);
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

			Entity newEntity = newScene->CreateEntity({}, id, name);
			enttMap[id] = newEntity;
		}

		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<MeshRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<DirectionalLightComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<PointLightComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		// Restore transforms
		auto transformView = dstSceneRegistry.view<TransformComponent>();
		for (auto e : transformView)
		{
			TransformComponent& tc = dstSceneRegistry.get<TransformComponent>(e);
			if (tc.Parent)
				tc.Parent = newScene->GetEntityByID(tc.Parent.GetComponent<IDComponent>().ID);
		}

		newScene->m_Skybox = other->m_Skybox;

		return newScene;
	}

	std::vector<ShaderUniform> Scene::GetGlobalUniforms(glm::vec3 cameraPos)
	{
		//Log.CoreInfo("X{0}Y{1}Z{2}", cameraPos[0], cameraPos[1], cameraPos[2]);
		std::vector<ShaderUniform> ret;
		ShaderUniform::UniformData data;

		// Vectors and transforms
		data.Vec3 = cameraPos;
		ret.push_back(ShaderUniform("u_CameraPosition", ShaderDataType::Float3, data));

		// Ambient light
		data.Vec3 = m_AmbientLight;
		ret.push_back(ShaderUniform("u_AmbientLightColor", ShaderDataType::Float3, data));
		// Ambient light intensity
		data.Float = m_AmbientLightIntensity;
		ret.push_back(ShaderUniform("u_AmbientLightIntensity", ShaderDataType::Float, data));

		return ret;
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