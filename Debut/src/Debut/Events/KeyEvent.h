#pragma once

#include "Event.h"

namespace Debut
{
	class DBT_API KeyEvent : public Event
	{
	protected:
		int m_KeyCode;

		KeyEvent(int keycode) : m_KeyCode(keycode) {}

	public:
		inline int GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(Keyboard | Input)
	};

	class DBT_API KeyPressedEvent : public KeyEvent
	{
	public:
		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class DTB_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

		std::string ToString const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

			EVENT_CLASS_TYPE(KeyReleased)
	};
}
