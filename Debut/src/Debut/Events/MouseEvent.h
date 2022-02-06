#pragma once

#include "Event.h"

namespace Debut
{
	class DBT_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		inline float GetX() { return m_MouseX; }
		inline float GetY() { return m_MouseY; }

		EVENT_CLASS_CATEGORY(Mouse | Input)
			EVENT_CLASS_TYPE(MouseMoved)
	private:
		float m_MouseX;
		float m_MouseY;

	};

	class DBT_API MouseScrollEvent : public Event
	{
	public:
		MouseScrollEvent(float offsetX, float offsetY) : m_OffsetX(offsetX), m_OffsetY(offsetY) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolled: " << m_OffsetX << ", " << m_OffsetY;
			return ss.str();
		}

		inline float GetOffsetX() { return m_OffsetX; }
		inline float GetOffsetY() { return m_OffsetY; }

		EVENT_CLASS_CATEGORY(Mouse | Input)
		EVENT_CLASS_TYPE(MouseScrolled)
	private:
		float m_OffsetX;
		float m_OffsetY;

	};

	class DBT_API MouseButtonEvent : public Event
	{
	public:
		inline int GetMouseButton() { return m_Button; }

		EVENT_CLASS_CATEGORY(Mouse | Input)
	protected:
		int m_Button;

		MouseButtonEvent(int mousebutton) : m_Button(mousebutton) {}
	};

	class DBT_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int mousebutton) : MouseButtonEvent(mousebutton) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressed: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
		
	};

	class DBT_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int mousebutton) : MouseButtonEvent(mousebutton) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleased: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}