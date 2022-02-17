#pragma once

#include "Debut/Events/ApplicationEvent.h"
#include "Debut/Events/MouseEvent.h"
#include "Debut/Events/KeyEvent.h"
#include "Debut/Layer.h"

namespace Debut
{
	class DBT_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0;
	};
}