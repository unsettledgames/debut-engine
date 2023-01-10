#include <Debut/dbtpch.h>
#include <Debut/Core/Log.h>

#include <Debut/Scripting/ScriptClass.h>
#include <Debut/Scripting/ScriptEngine.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Debut
{
	ScriptClass::ScriptClass(const std::string& name) : m_ClassName(name)
	{
        MonoImage* image = mono_assembly_get_image(ScriptEngine::s_Data.CoreAssembly);
        MonoClass* klass = mono_class_from_name(image, "Debut", name.c_str());

        if (klass == nullptr)
        {
            Log.CoreError("Couldn't retrieve class {0} from C# assembly", name);
            return;
        }

        m_Class = klass;
        m_OnStart = mono_class_get_method_from_name(klass, "OnStart", 0);
        m_OnUpdate = mono_class_get_method_from_name(klass, "OnUpdate", 1);
	}
}