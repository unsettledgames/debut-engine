#pragma once

#include <Debut/Core/Layer.h>
#include <imgui.h>
#include <string>

namespace Debut
{
	class Event;

	struct FontIcon
	{
		wchar_t Character;
		std::string TexturePath;
	};
	
	struct WindowStyle
	{
		float BorderSize = 0.0f;
		float PopupBorderSize = 0.0f;

		ImVec2 TitleAlign = { 0.5f, 0.5f };
		ImVec2 DisplaySafeArea = { 0, 6 };
		float Rounding = 2;

	};
	struct FrameStyle
	{
		float BorderSize = 0;
		float Rounding = 2;
	};
	struct TabStyle
	{
		float TabRounding = 2;
	};
	struct ScrollbarStyle
	{
		float ScrollbarSize = 16;
		float ScrollBarRounding = 6;
		float GrabRounding = 2;
	};
	struct Style
	{
		WindowStyle Window;
		FrameStyle Frame;
		TabStyle Tab;

		ImVec2 ItemInnerSpacing = { 6, 4 };
		float IndentSpacing = 16;
	};

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
		std::vector<FontIcon> GetFontIcons();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0;
	};
}