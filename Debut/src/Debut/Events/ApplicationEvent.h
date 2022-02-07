#pragma once

#include "../Core.h"
#include "Event.h"

namespace Debut
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		inline float GetWidth() const { return m_Width; }
		inline float GetHeight() const { return m_Height; }

		EVENT_CLASS_CATEGORY(Application)
			EVENT_CLASS_TYPE(WindowResize)

	private:
		unsigned int m_Width;
		unsigned int m_Height;
	};

	class WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent(unsigned int x, unsigned int y) : m_x(x), m_y(y) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMovedEvent: " << m_x << ", " << m_y;
			return ss.str();
		}

		inline float GetX() const { return m_x; }
		inline float GetY() const { return m_y; }

		EVENT_CLASS_CATEGORY(Application)
		EVENT_CLASS_TYPE(WindowMoved)

	private:
		unsigned int m_x;
		unsigned int m_y;
	};

	class WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowFocusEvent";
			return ss.str();
		}


		EVENT_CLASS_CATEGORY(Application)
			EVENT_CLASS_TYPE(WindowFocus)
	};

	class WindowLostFocusEvent : public Event
	{
	public:
		WindowLostFocusEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowLostFocusEvent";
			return ss.str();
		}


		EVENT_CLASS_CATEGORY(Application)
			EVENT_CLASS_TYPE(WindowLostFocus)
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowCloseEvent";
			return ss.str();
		}


		EVENT_CLASS_CATEGORY(Application)
			EVENT_CLASS_TYPE(WindowClose)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppTickEvent";
			return ss.str();
		}


		EVENT_CLASS_CATEGORY(Application)
		EVENT_CLASS_TYPE(AppTick)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppUpdateEvent";
			return ss.str();
		}


		EVENT_CLASS_CATEGORY(Application)
			EVENT_CLASS_TYPE(AppUpdate)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppRenderEvent";
			return ss.str();
		}


		EVENT_CLASS_CATEGORY(Application)
		EVENT_CLASS_TYPE(AppRender)
	};
}