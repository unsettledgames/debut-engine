#pragma once

extern "C"
{
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoAssembly MonoAssembly;
}

namespace Debut
{
	struct ScriptComponent;
	
	class Entity;
	class ScriptClass;
	class ScriptInstance;

	struct ScriptEngineData
	{
		MonoDomain* RootDomain;
		MonoDomain* AppDomain;

		MonoImage* CoreImage;
		MonoAssembly* CoreAssembly;

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

	private:
		static ScriptEngineData s_Data;

	};
}