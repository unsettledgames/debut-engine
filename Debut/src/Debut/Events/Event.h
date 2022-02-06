#pragma once

#include "../Core.h"

#include <string>
#include <sstream>
#include <functional>

namespace Debut
{
	// At the moment, Events are totally synchronous, that means that events are immediately dispatched as soon as
	// they happen. IMPROVE: use bufferization 
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRenderer,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	// Used to filter events (e.g. only listen to keyboard events)
	enum EventCategory
	{
		None = 0,
		Application =	BIT(0),
		Input =			BIT(1),
		Keyboard =		BIT(2),
		Mouse =			BIT(3),
		MouseButton =	BIT(4)
	};

#define EVENT_CLASS_TYPE(type)	static EventType GetStaticType() { return EventType::##type; } \
								virtual EventType GetEventType() { return GetStaticType(); } \
								virtual const char* GetName() const override { return #type;}

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override {return category;}

	class DBT_API Event
	{
		friend class EventDispatcher;

	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	protected:
		// A layer may stop event propagation
		bool m_Handled = false;
	};

	class EventDispatcher
	{
	private:
	public:
	};
}