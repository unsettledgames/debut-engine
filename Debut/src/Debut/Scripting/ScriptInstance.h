#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>

extern "C" 
{
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoObject MonoObject;
}

namespace Debut
{
	class UUID;
	class ScriptClass;

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> klass, UUID& entityID);

		void InvokeOnStart();
		void InvokeOnUpdate(float ts);

	private:
		Ref<ScriptClass> m_Class;
		UUID m_EntityID;
		
		MonoObject* m_Object;
	};
}