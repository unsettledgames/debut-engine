#include <Debut/dbtpch.h>
#include <Debut/Scripting/ScriptEngine.h>
#include <Debut/Scripting/ScriptInstance.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Debut
{
	ScriptInstance::ScriptInstance(Ref<ScriptClass> klass) : m_Class(klass)
	{
		m_Object = mono_object_new(ScriptEngine::s_Data.AppDomain, klass->m_Class);
		mono_runtime_object_init(m_Object);
	}

	void ScriptInstance::InvokeOnStart()
	{
		if (m_Class->m_OnStart == nullptr)
			return;

		MonoObject* exception = nullptr;
		mono_runtime_invoke(m_Class->m_OnStart, m_Class->m_Class, nullptr, &exception);	
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_Class->m_OnUpdate == nullptr)
			return;

		MonoObject* exception = nullptr;
		void* param = { &ts };
		mono_runtime_invoke(m_Class->m_OnUpdate, m_Class->m_Class, &param, &exception);
	}
}