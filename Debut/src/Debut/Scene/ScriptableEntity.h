#pragma once

#include "Debut/Scene/Entity.h"

namespace Debut
{
	class ScriptableEntity
	{
	friend class Scene;

	public:
		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent();
		}

	private:
		Entity m_Entity;
		
	};
}