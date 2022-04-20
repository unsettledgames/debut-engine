#pragma once

#include "Debut/Scene/Entity.h"

namespace Debut
{
	class ScriptableEntity
	{
	friend class Scene;

	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

	protected:
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnCreate() {}
		virtual void OnDestroy() {}

	private:
		Entity m_Entity;
		
	};
}