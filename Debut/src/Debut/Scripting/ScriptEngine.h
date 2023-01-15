#pragma once

extern "C"
{
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoClass MonoClass;
}

namespace Debut
{
	struct ScriptComponent;
	
	class Entity;
	class Scene;
	class ScriptClass;
	class ScriptInstance;

	struct ScriptEngineData
	{
		MonoDomain* RootDomain;
		MonoDomain* AppDomain;

		MonoImage* CoreImage;
		MonoAssembly* CoreAssembly;

		MonoClass* EntityClass;

		// Class name -> ScriptClass
		std::unordered_map <std::string, Ref<ScriptClass>> Classes;
		// Entity -> script instance
		std::unordered_map<uint32_t, Ref<ScriptInstance>> ScriptInstances;
	};

	class ScriptEngine
	{
		friend class ScriptClass;
		friend class ScriptInstance;

	public:
		static void Init();
		static void Shutdown();

		static void Instantiate(ScriptComponent& script, Entity& entity);

		static void CallOnStart();
		static void CallOnUpdate(float ts);

		inline static Scene* GetContext() { return s_Context; }
		inline static void SetContext(Scene* scene) { s_Context = scene; }

	private:
		static ScriptEngineData s_Data;
		static Scene* s_Context;

	};
}