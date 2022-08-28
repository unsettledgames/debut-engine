#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Debut/Scene/SceneCamera.h"
#include <Debut/Scene/ScriptableEntity.h>

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
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		Entity Parent = {};

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::vec3& translation) : Translation(translation) {}

		glm::mat4 GetTransform()
		{
			// Get the parent matrix
			glm::mat4 worldMatrix;
			if (!Parent)
				worldMatrix = glm::mat4(1.0);
			else
				worldMatrix = Parent.Transform().GetTransform();

			// Compose the local matrix
			glm::mat4 transform(1.0f);
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			
			// Apply the parent matrix
			return worldMatrix * (glm::translate(transform, Translation) 
				* rotation 
				* glm::scale(glm::mat4(1.0f), Scale));
		}

		glm::mat4 GetLocalTransform()
		{
			// Compose the local matrix
			glm::mat4 transform(1.0f);
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			// Apply the parent matrix
			return (glm::translate(transform, Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale));
		}

		void SetParent(Entity parent) 
		{ 
			glm::mat4 finalTransform = GetLocalTransform();
			glm::quat rotation;
			glm::vec3 skew;
			glm::vec4 persp;

			if (Parent)
				finalTransform = Parent.Transform().GetTransform() * finalTransform;
			if (parent)
				finalTransform = glm::inverse(parent.Transform().GetTransform()) * finalTransform;

			glm::decompose(finalTransform, Scale, rotation, Translation, skew, persp);
			Rotation = glm::eulerAngles(rotation);

			Parent = parent; 
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
		UUID Texture = 0;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() : Color(glm::vec4(1.0f)) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
	};

	struct MeshRendererComponent
	{
		UUID Material = 0;
		UUID Mesh = 0;

		bool Instanced = false;

		MeshRendererComponent()  {}
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(UUID mesh, UUID material) : Material(material), Mesh(mesh), Instanced(false) {}
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

			return BodyType::Dynamic;
		}
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 1.0f, 1.0f };

		UUID Material = 0;

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
}