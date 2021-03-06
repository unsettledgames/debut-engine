#pragma once

#include "Debut/Events/ApplicationEvent.h"
#include "Debut/Events/MouseEvent.h"
#include "Debut/Events/KeyEvent.h"
#include "Debut/Core/Layer.h"

namespace Debut
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnEvent(Event& e) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

		inline void SetBlockEvents(bool block) { m_BlockEvents = block; }
		
		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0;
	};
}