#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Scripting/ScriptClass.h>

extern "C" 
{
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoObject MonoObject;
}

namespace Debut
{
	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> klass);

		void InvokeOnStart();
		void InvokeOnUpdate(float ts);

	private:
		Ref<ScriptClass> m_Class;
		
		MonoObject* m_Object;
		MonoMethod* m_OnStartMethod;
		MonoMethod* m_OnUpdateMethod;
	};
}