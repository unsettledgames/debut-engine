#pragma once

#include <string>

namespace Debut
{
	class Event;
	class Timestep;

	class Layer
	{
	public:
		Layer(const std::string& name = "UnnamedLayer");
		virtual ~Layer();

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate(Timestep& ts) {};
		virtual void OnEvent(Event& e) {};
		virtual void OnImGuiRender() {};

		inline const std::string& GetName() { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};
}
