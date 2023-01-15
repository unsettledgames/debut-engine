#include <Debut/dbtpch.h>
#include <Debut/Scripting/ScriptEngine.h>

#include <Debut/Core/Core.h>
#include <Debut/Scene/Components.h>
#include <Debut/Scene/Entity.h>

#include <Debut/Scripting/ScriptGlue.h>
#include <Debut/Scripting/ScriptClass.h>
#include <Debut/Scripting/ScriptInstance.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

// TODO: add support for multiple scripts in same entity

namespace Debut
{
    ScriptEngineData ScriptEngine::s_Data;
    Scene* ScriptEngine::s_Context;

    char* ReadBytes(const std::string& filepath, uint32_t* outSize)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream)
            return nullptr;

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        uint32_t size = end - stream.tellg();

        if (size == 0)
            return nullptr;

        char* buffer = new char[size];
        stream.read((char*)buffer, size);
        stream.close();

        *outSize = size;
        return buffer;
    }

    MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
    {
        uint32_t fileSize = 0;
        char* fileData = ReadBytes(assemblyPath, &fileSize);

        // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            Log.CoreError("Mono image error: {0}", std::string(mono_image_strerror(status)));
            return nullptr;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
        mono_image_close(image);

        delete[] fileData;
        return assembly;
    }

    void PrintAssemblyTypes(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            printf("%s.%s\n", nameSpace, name);
        }
    }

	void ScriptEngine::Init()
	{
        DBT_PROFILE_FUNCTION();
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

        // Load the assembly
        std::string assemblyPath = "../../../Resources/Mono/Debut";
#ifdef DBT_DEBUG
        assemblyPath += "/Debug/DebutScriptingd.dll";
#else
        assemblyPath += "/Release/DebutScripting.dll";
#endif

        s_Data.CoreAssembly = LoadCSharpAssembly(assemblyPath);
        s_Data.CoreImage = mono_assembly_get_image(s_Data.CoreAssembly);
        PrintAssemblyTypes(s_Data.CoreAssembly);

        s_Data.EntityClass = mono_class_from_name(ScriptEngine::s_Data.CoreImage, "Debut", "Entity");

        ScriptGlue::RegisterFunctions();
	}

    void ScriptEngine::Shutdown()
    {
        mono_jit_cleanup(s_Data.RootDomain);

        s_Data.RootDomain = nullptr;
        s_Data.AppDomain = nullptr;
    }

    void ScriptEngine::Instantiate(ScriptComponent& script, Entity& entity)
    {
        Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(script.ClassName);
        Ref<ScriptInstance> scriptInstance = CreateRef<ScriptInstance>(scriptClass, (UUID)entity.ID());

        s_Data.ScriptInstances[entity] = scriptInstance;
    }

    void ScriptEngine::CallOnStart()
    {
        // Call OnStart on every instance
        for (auto& entt : s_Data.ScriptInstances)
            entt.second->InvokeOnStart();
    }

    void ScriptEngine::CallOnUpdate(float ts)
    {
        for (auto& entt : s_Data.ScriptInstances)
            entt.second->InvokeOnUpdate(ts);
    }
}