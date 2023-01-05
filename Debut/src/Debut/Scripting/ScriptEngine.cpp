#include <Debut/dbtpch.h>
#include <Debut/Core/Core.h>
#include <Debut/Scripting/ScriptEngine.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Debut
{
    struct ScriptEngineData
    {
        MonoDomain* RootDomain;
        MonoDomain* AppDomain;
    };

    ScriptEngineData s_Data;

	void ScriptEngine::Init()
	{
		mono_set_assemblies_path("../../../Resources/Mono/lib/4.5");

        MonoDomain* rootDomain = mono_jit_init("DebutScriptRuntime");
        if (rootDomain == nullptr)
        {
            Log.CoreError("Couldn't create Mono root domain");
            return;
        }

        // Store the root domain pointer
        s_Data.RootDomain = rootDomain;

        // Create an App Domain
        s_Data.AppDomain = mono_domain_create_appdomain("DebutScriptCore", nullptr);
        mono_domain_set(s_Data.AppDomain, true);
	}
}