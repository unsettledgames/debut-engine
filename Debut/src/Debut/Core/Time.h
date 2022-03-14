#pragma once

namespace Debut
{
	class Timestep
	{
	public:
		Timestep(float time = 0.0f) : m_Time(time)
		{

		}

		operator float() const { return m_Time; }

		float GetSeconds() { return m_Time; }
		float GetMilliseconds() { return m_Time * 1000; }
	private:
		float m_Time;
	};
}