#pragma once

#include <string>
#include <unordered_map>

extern "C" 
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoMethod MonoMethod;
}

namespace Debut
{
	class ScriptClass
	{
		friend class ScriptInstance;

	public:
		ScriptClass(const std::string& name);

	private:
		std::string m_ClassName;
		MonoClass* m_Class;
		std::unordered_map<std::string, std::vector<MonoMethod*>> m_Methods;

		MonoMethod* m_OnStart;
		MonoMethod* m_OnUpdate;
	};
}