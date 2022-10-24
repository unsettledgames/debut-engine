#include "Debut/dbtpch.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include "Debut/Scene/Entity.h"
#include "Debut/Scene/Components.h"
#include "Debut/Rendering/Shader.h"
#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Rendering/Renderer/Renderer.h>
#include "Debut/Rendering/Renderer/Renderer2D.h"
#include "Debut/Rendering/Renderer/Renderer3D.h"
#include "Debut/AssetManager/AssetManager.h"
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Physics/PhysicsSystem3D.h>
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
		delete m_PhysicsSystem3D;
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
	void Scene::OnComponentAdded(Rigidbody3DComponent& rb3d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(BoxCollider2DComponent& bc2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(PolygonCollider2DComponent& pc2d, Entity entity) {}
	template<>
	void Scene::OnComponentAdded(CircleCollider2DComponent& bc2d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(BoxCollider3DComponent& bc3d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(SphereCollider3DComponent& sc3d, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(MeshCollider3DComponent& sc3d, Entity entity)
	{
		// Automatically add the mesh if the object has a mesh renderer
		if (entity.HasComponent<MeshRendererComponent>())
		{
			MeshRendererComponent& meshRenderer = entity.GetComponent<MeshRendererComponent>();
			sc3d.Mesh = meshRenderer.Mesh;
		}
	}
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
		{
			Component& c = dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			if (dst.HasComponent<IDComponent>())
				c.Owner = dst.GetComponent<IDComponent>().ID;
		}

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
			Renderer3D::DrawModel(mesh, transform.GetTransform(), (int)entity);
		}

		Renderer3D::EndScene();

		// 2D Rendering
		Renderer2D::BeginScene(camera, glm::inverse(camera.GetView()));

		auto group2D = m_Registry.group<TransformComponent, SpriteRendererComponent>();
		for (auto entity : group2D)
		{
			auto& [transform, sprite] = group2D.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		}

		Renderer2D::EndScene();

		if (m_SceneConfig.RenderColliders)
		{
			RendererDebug::BeginScene(camera, transform);
			
			// 3D Physics colliders
			auto rigidbody3DGroup = m_Registry.view<Rigidbody3DComponent>();
			for (auto entity : rigidbody3DGroup)
			{
				Entity e = { entity, this };
				auto& objTransform = e.Transform();

				if (e.HasComponent<BoxCollider3DComponent>())
				{
					auto& collider = e.GetComponent<BoxCollider3DComponent>();
					RendererDebug::DrawBox(collider.Size, collider.Offset, objTransform.GetTransform(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				}
				else if (e.HasComponent<SphereCollider3DComponent>())
				{
					glm::vec3 trans, rot, scale;
					glm::mat4 objTransformMat = objTransform.GetTransform();
					MathUtils::DecomposeTransform(objTransformMat, trans, rot, scale);

					auto& collider = e.GetComponent<SphereCollider3DComponent>();
					RendererDebug::DrawSphere(collider.Radius, collider.Offset, rot, scale, glm::inverse(transform), objTransformMat);
				}
				else if (e.HasComponent<MeshCollider3DComponent>())
				{
					auto& collider = e.GetComponent<MeshCollider3DComponent>();
					RendererDebug::DrawMesh(collider.Mesh, collider.Offset, objTransform.GetTransform(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				}
			}

			// 2D Physics colliders
			auto rigidbody2DGroup = m_Registry.view<Rigidbody2DComponent>();
			for (auto entity : rigidbody2DGroup)
			{
				Entity e = { entity, this };
				auto& objTransform = e.Transform();

				if (e.HasComponent<BoxCollider2DComponent>())
				{
					auto& collider = e.GetComponent<BoxCollider2DComponent>();
					RendererDebug::DrawRect(objTransform.GetTransform(), collider.Size, collider.Offset, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				}
				else if (e.HasComponent<CircleCollider2DComponent>())
				{
					auto& collider = e.GetComponent<CircleCollider2DComponent>();
					RendererDebug::DrawCircle(collider.Radius, glm::vec3(collider.Offset, objTransform.Translation.z), 
						objTransform.GetTransform(), 40);
				}
				else if (e.HasComponent<PolygonCollider2DComponent>())
				{
					auto& collider = e.GetComponent<PolygonCollider2DComponent>();
					RendererDebug::DrawPolygon(collider.GetTriangles(), glm::vec3(collider.Offset, objTransform.Translation.z),
						objTransform.GetTransform(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				}
			}
			
			RendererDebug::EndScene();
		}
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

		// Update 3D physics
		{
			DBT_PROFILE_SCOPE("Scene: Physics3D update");
			// Step the world
			m_PhysicsSystem3D->Step(ts);

			// Stuff moved, update the transforms to reflect that
			auto view = m_Registry.view<Rigidbody3DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				Rigidbody3DComponent& body = entity.GetComponent<Rigidbody3DComponent>();
				TransformComponent& transform = entity.Transform();

				m_PhysicsSystem3D->UpdateBody(entity.Transform(), body, *((BodyID*)body.RuntimeBody));
				transform.Translation -= glm::vec3(glm::mat4(glm::quat(transform.Rotation)) *
					glm::vec4(transform.Scale * body.ShapeOffset, 1.0f));
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
					Renderer3D::DrawModel(mesh, transform.GetTransform(), (int)entity);
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
		Physics3DSettings defaultSettings;
		// TODO: physics settings
		m_PhysicsWorld2D = new b2World({b2Vec2(0.0f, -9.8f)});
		m_PhysicsSystem3D = new PhysicsSystem3D(defaultSettings);

		auto rigidbodyView2D = m_Registry.view<Rigidbody2DComponent>();
		auto rigidbodyView3D = m_Registry.view<Rigidbody3DComponent>();

		// Create Rigidbodies
		for (auto e : rigidbodyView2D)
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
				boxShape.SetAsBox(transform.Scale.x * bc2d.Size.x / 2, transform.Scale.y * bc2d.Size.y / 2, b2Vec2(transform.Scale.x * bc2d.Offset.x, transform.Scale.y * bc2d.Offset.y), 0);

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

			if (entity.HasComponent<PolygonCollider2DComponent>())
			{
				auto& pc2d = entity.GetComponent<PolygonCollider2DComponent>();
				Ref<PhysicsMaterial2D> material = AssetManager::Request<PhysicsMaterial2D>(pc2d.Material);
				auto& triangles = pc2d.GetTriangles();

				// Add a triangular shape to form a polygon
				for (auto& triangle : triangles)
				{
					// Set the points
					b2PolygonShape polygonShape;
					b2Vec2 points[3];
					for (uint32_t i = 0; i < 3; i++)
						points[i] = { triangle[i].x + pc2d.Offset.x, triangle[i].y + pc2d.Offset.y };
					polygonShape.Set(points, 3);

					// Create the fixture
					b2FixtureDef fixtureDef;
					fixtureDef.shape = &polygonShape;

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

		// Create 3D rigidbodies
		for (auto e : rigidbodyView3D)
		{
			Entity entity = { e, this };
			auto& transform = entity.Transform();
			Rigidbody3DComponent& component = entity.GetComponent<Rigidbody3DComponent>();

			// Create the actual body
			if (entity.HasComponent<BoxCollider3DComponent>())
			{
				BoxCollider3DComponent& collider = entity.GetComponent<BoxCollider3DComponent>();
				BodyID* body = m_PhysicsSystem3D->CreateBoxColliderBody(collider, component, transform);
				
				// Save the body pointer
				component.RuntimeBody = (void*)body;
				component.ShapeOffset = collider.Offset;
			}
			else if (entity.HasComponent<SphereCollider3DComponent>())
			{
				SphereCollider3DComponent& collider = entity.GetComponent<SphereCollider3DComponent>();
				BodyID* body = m_PhysicsSystem3D->CreateSphereColliderBody(collider, component, transform);
				
				component.RuntimeBody = (void*)body;
				component.ShapeOffset = collider.Offset;
			}
			else if (entity.HasComponent<MeshCollider3DComponent>())
			{
				MeshCollider3DComponent& collider = entity.GetComponent<MeshCollider3DComponent>();
				BodyID* body = m_PhysicsSystem3D->CreateMeshColliderBody(collider, component, transform);

				component.RuntimeBody = (void*)body;
				component.ShapeOffset = collider.Offset;
			}
		}

		// Begin 3D physics simulation
		m_PhysicsSystem3D->Begin();
	}

	void Scene::OnRuntimeStop()
	{
		delete m_PhysicsWorld2D;
		m_PhysicsWorld2D = nullptr;

		m_PhysicsSystem3D->End();
	}

	Entity Scene::DuplicateEntity(Entity& entity, Entity& parent)
	{
		if (!entity)
			return {};
		Entity duplicate = CreateEmptyEntity();
		
		CopyComponentIfExists<TagComponent>(duplicate, entity);
		duplicate.GetComponent<TagComponent>().Name += " Copy";
		CopyComponentIfExists<TransformComponent>(duplicate, entity);
		CopyComponentIfExists<SpriteRendererComponent>(duplicate, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(duplicate, entity);
		CopyComponentIfExists<Rigidbody3DComponent>(duplicate, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(duplicate, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(duplicate, entity);
		CopyComponentIfExists<PolygonCollider2DComponent>(duplicate, entity);
		CopyComponentIfExists<BoxCollider3DComponent>(duplicate, entity);
		CopyComponentIfExists<SphereCollider3DComponent>(duplicate, entity);
		CopyComponentIfExists<MeshCollider3DComponent>(duplicate, entity);
		CopyComponentIfExists<CameraComponent>(duplicate, entity);
		CopyComponentIfExists<NativeScriptComponent>(duplicate, entity);
		CopyComponentIfExists<MeshRendererComponent>(duplicate, entity);
		CopyComponentIfExists<DirectionalLightComponent>(duplicate, entity);
		CopyComponentIfExists<PointLightComponent>(duplicate, entity);
		
		auto& transform = entity.Transform();
		duplicate.Transform().SetParent(parent);

		// Refill children
		for (uint32_t i = 0; i < transform.Children.size(); i++)
			Entity childDup = DuplicateEntity(Entity::s_ExistingEntities[transform.Children[i]], duplicate);

		return duplicate;
	}

	Entity Scene::CreateEmptyEntity()
	{
		Entity ret = { m_Registry.create(), this };

		IDComponent id = ret.AddComponent<IDComponent>();
		Entity::s_ExistingEntities[id.ID] = ret;

		return ret;
	}

	Entity Scene::CreateEntity(Entity parent, const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };

		IDComponent id = ret.AddComponent<IDComponent>();
		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);
		ret.Transform().SetParent(parent);

		Entity::s_ExistingEntities[id.ID] = ret;

		return ret;
	}

	Entity Scene::CreateEntity(Entity parent, const UUID& id, const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };

		IDComponent& idC = ret.AddComponent<IDComponent>();
		idC.ID = id;
		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);
		ret.Transform().SetParent(parent);

		Entity::s_ExistingEntities[id] = ret;

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
		entity.Transform().SetParent({});
		Entity::s_ExistingEntities.erase(entity.ID());
		m_Registry.destroy(entity);
	}

	void Scene::SetSkybox(UUID skybox)
	{
		m_Skybox = AssetManager::Request<Skybox>(skybox);
	}

	void Scene::SetSceneConfig(const SceneConfig& config)
	{
		if (config != m_SceneConfig)
		{
			m_SceneConfig = config;
			Renderer::ToggleTextures(m_SceneConfig.RenderSurfaces);
			Renderer::ToggleWireframe(m_SceneConfig.RenderWireframe);
		}
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
		CopyComponent<Rigidbody3DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<PolygonCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider3DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SphereCollider3DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<MeshCollider3DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
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