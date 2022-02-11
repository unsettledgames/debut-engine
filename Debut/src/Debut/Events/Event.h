#pragma once

#include "Debut/dbtpch.h"
#include "../Core.h"

namespace Debut
{
	// At the moment, Events are totally synchronous, that means that events are immediately dispatched as soon as
	// they happen. IMPROVE: use bufferization 
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	// Used to filter events (e.g. only listen to keyboard events)
	enum EventCategory
	{
		None = 0,
		ApplicationEvent =		BIT(0),
		InputEvent =			BIT(1),
		KeyboardEvent =			BIT(2),
		MouseEvent =			BIT(3),
		MouseButtonEvent =		BIT(4)
	};

#define EVENT_CLASS_TYPE(type)	static EventType GetStaticType()				{ return EventType::##type; } \
								virtual EventType GetEventType() const override { return GetStaticType(); } \
								virtual const char* GetName() const override	{ return #type;}

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override {return category;}

	class DBT_API Event
	{
		friend class EventDispatcher;

	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) const { return GetCategoryFlags() & category; }
		inline bool Handled() const { return m_Handled; }
	protected:
		// A layer may stop event propagation
		bool m_Handled = false;
	};

	class EventDispatcher
	{
		template <typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event) : m_Event(event) {}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}

			return false;
		}

	private:
		Event& m_Event;
	};
}