#pragma once

#include "Debut/Core.h"
#include "Events/Event.h"

namespace Debut
{
	class DBT_API Layer
	{
	public:
		Layer(const std::string& name = "UnnamedLayer");
		virtual ~Layer();

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate() {};
		virtual void OnEvent(Event& e) {};
		virtual void OnImGuiRender() {};

		inline const std::string& GetName() { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};
}
