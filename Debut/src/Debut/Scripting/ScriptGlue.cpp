#include <Debut/dbtpch.h>
#include <Debut/Scripting/ScriptEngine.h>
#include <Debut/Scripting/ScriptGlue.h>

#include <Debut/Core/Input.h>
#include <Debut/Core/KeyCodes.h>

#include <Debut/Scene/Entity.h>
#include <Debut/Scene/Scene.h>
#include <Debut/Scene/Components.h>

#include "mono/metadata/reflection.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>

namespace Debut
{

#pragma region Input
	
	static bool Input_IsKeyDown(int key)
	{
		return Input::IsKeyPressed(key);
	}

#pragma endregion Input

#pragma region Entity

	bool Entity_HasComponent(uint64_t entity, MonoReflectionType* type)
	{
		Entity e = Entity::s_ExistingEntities[entity];

		MonoType* managedType = mono_reflection_type_get_type(type);
		std::string typeName = mono_type_get_name(managedType);

		if (!typeName.compare("Debut.Transform")) return e.HasComponent<TransformComponent>();

		return false;
	}

#pragma endregion Entity

#pragma region TransformComponent

	static void TransformComponent_GetTranslation(uint64_t entity, glm::vec3& out)
	{
		Scene* scene = ScriptEngine::GetContext();
		Entity e = Entity::s_ExistingEntities[entity];
		
		TransformComponent& tc = e.GetComponent<TransformComponent>();
		out = tc.GetTranslation();
	}

	static void TransformComponent_SetTranslation(uint64_t entity, glm::vec3& in)
	{
		Scene* scene = ScriptEngine::GetContext();
		Entity e = Entity::s_ExistingEntities[entity];
		
		TransformComponent& tc = e.GetComponent<TransformComponent>();
		tc.SetTranslation(in);
	}

	static void TransformComponent_GetEulerRotation(uint64_t entity, glm::vec3& out)
	{
		Scene* scene = ScriptEngine::GetContext();
		Entity e = Entity::s_ExistingEntities[entity];
		TransformComponent& tc = e.GetComponent<TransformComponent>();

		out = tc.GetRotation();
	}

	static void TransformComponent_SetEulerRotation(uint64_t entity, glm::vec3& in)
	{
		Scene* scene = ScriptEngine::GetContext();
		Entity e = Entity::s_ExistingEntities[entity];
		TransformComponent& tc = e.GetComponent<TransformComponent>();

		tc.Rotation = in;
	}

	static void TransformComponent_GetScale(uint64_t entity, glm::vec3& out)
	{
		Scene* scene = ScriptEngine::GetContext();
		Entity e = Entity::s_ExistingEntities[entity];
		TransformComponent& tc = e.GetComponent<TransformComponent>();

		out = tc.GetTransform() * glm::vec4(tc.Scale, 0.0f);
	}

	static void TransformComponent_SetScale(uint64_t entity, glm::vec3& in)
	{
		Scene* scene = ScriptEngine::GetContext();
		Entity e = Entity::s_ExistingEntities[entity];
		TransformComponent& tc = e.GetComponent<TransformComponent>();
		tc.SetScale(in);
	}

#pragma endregion TransformComponent
	

	void ScriptGlue::RegisterFunctions()
	{
		mono_add_internal_call("Debut.Core::" "Input_IsKeyDown", Input_IsKeyDown);

		mono_add_internal_call("Debut.Core::" "Entity_HasComponent", Entity_HasComponent);

		mono_add_internal_call("Debut.Core::" "TransformComponent_GetTranslation", TransformComponent_GetTranslation);
		mono_add_internal_call("Debut.Core::" "TransformComponent_SetTranslation", TransformComponent_SetTranslation);
		mono_add_internal_call("Debut.Core::" "TransformComponent_GetEulerRotation", TransformComponent_GetEulerRotation);
		mono_add_internal_call("Debut.Core::" "TransformComponent_SetEulerRotation", TransformComponent_SetEulerRotation);
		mono_add_internal_call("Debut.Core::" "TransformComponent_GetScale", TransformComponent_GetScale);
		mono_add_internal_call("Debut.Core::" "TransformComponent_SetScale", TransformComponent_SetScale);
	}
}