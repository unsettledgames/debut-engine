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
        MonoAssembly* CoreAssembly;
    };

    ScriptEngineData s_Data;

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

    MonoClass* GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        MonoClass* klass = mono_class_from_name(image, namespaceName, className);

        if (klass == nullptr)
        {
            // Log error here
            return nullptr;
        }

        return klass;
    }

    void TestMethods()
    {
        MonoClass* testClass = GetClassInAssembly(s_Data.CoreAssembly, "", "Test");
        MonoObject* classInstance = mono_object_new(s_Data.AppDomain, testClass);
        mono_runtime_object_init(classInstance);

        // Test print method
        MonoMethod* printMethod = mono_class_get_method_from_name(testClass, "Print", 0);
        MonoObject* exception = nullptr;
        mono_runtime_invoke(printMethod, classInstance, nullptr, &exception);
        
        // Test set float method
        float value = 32.0f;
        void* params = { &value };
        MonoMethod* setFloatMethod = mono_class_get_method_from_name(testClass, "SetFloat", 1);
        mono_runtime_invoke(setFloatMethod, classInstance, &params, &exception);
        mono_runtime_invoke(printMethod, classInstance, nullptr, &exception);

        // Test get float method

    }

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

        // Load the assembly
        std::string assemblyPath = "../../../Resources/Mono/Debut";
#ifdef DBT_DEBUG
        assemblyPath += "/Debug/DebutScriptingd.dll";
#else
        assemblyPath += "/Release/DebutScripting.dll";
#endif

        s_Data.CoreAssembly = LoadCSharpAssembly(assemblyPath);
        PrintAssemblyTypes(s_Data.CoreAssembly);
        TestMethods();
	}

    void ScriptEngine::Shutdown()
    {
        mono_jit_cleanup(s_Data.RootDomain);

        s_Data.RootDomain = nullptr;
        s_Data.AppDomain = nullptr;
    }
}