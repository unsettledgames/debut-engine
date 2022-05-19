#pragma once

#include "Debut/Core/Time.h"
#include "Debut/Scene/SceneCamera.h"
#include <glm/glm.hpp>
#include <Debut/Scene/ScriptableEntity.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <Debut/Renderer/Texture.h>
#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>

namespace Debut
{
	// BASIC

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;
		std::string Name;

		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) : Tag("Untagged"), Name(name) {}
		TagComponent(const std::string& name, const std::string& tag) : Tag(tag), Name(name) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation;
		glm::vec3 Rotation = glm::vec3(0,0,0);
		glm::vec3 Scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation) : Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 transform(1.0f);
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(transform, Translation) 
				* rotation 
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	// RENDERING

	struct CameraComponent
	{
		Debut::SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color;
		Ref<Texture2D> Texture = nullptr;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() : Color(glm::vec4(1.0f)) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
	};

	// PHYSICS AND COLLIDERS
	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };

		BodyType Type = BodyType::Static;
		// TODO: constraints
		bool FixedRotation = false;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

		static BodyType StrToRigidbody2DType(const std::string& type)
		{
			if (type == "Static") return BodyType::Static;
			if (type == "Dynamic") return BodyType::Dynamic;
			if (type == "Kinematic") return BodyType::Kinematic;
		}
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 1.0f, 1.0f };

		// Move to physics material
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.5f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 1.0f;
		
		// Move to physics material
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.5f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	// SCRIPT
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()	
		{
			InstantiateScript = []() {return static_cast<ScriptableEntity*>(new T()); };	   
			DestroyScript = [](NativeScriptComponent* nsc) {delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	// Utility functions
	namespace Components
	{
		/*
		struct ComponentData
		{
			const std::string Name;
		};

		static uint32_t NComponents = 9;
		enum class ComponentType
		{
			ID, Tag, Transform,
			Camera, SpriteRenderer, 
			Rigidbody2D, BoxCollider2D, CircleCollider2D,
			NativeScript
		};

		typedef struct
		{
			ComponentType Type;
			std::string Name;
		} ComponentTypeDef;

		typedef struct : ComponentTypeDef
		{
			ComponentType Type = ComponentType::ID;
			std::string Name = "IDComponent";
		}IDType;

		typedef struct : ComponentTypeDef
		{
			ComponentType Type = ComponentType::Tag;
			std::string Name = "TagComponent";
		}TagType;

		typedef struct : ComponentTypeDef
		{
			ComponentType Type = ComponentType::Transform;
			std::string Name = "TransformComponent";
		}TransformType;

		static ComponentTypeDef Components[] =
		{
			IDType(),
			TagType(),
			TransformType()
		};
		*/
	}
}