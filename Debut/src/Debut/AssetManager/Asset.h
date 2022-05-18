#pragma once

#include <Debut/Core/UUID.h>

namespace Debut
{
	template <typename T>
	class Asset
	{
	public:
		Asset(T payload) : m_Payload(payload) { m_ID = UUID(); }

		UUID GetID() { return m_ID; }
		T GetPayload() { return m_Payload; }

	private:
		UUID m_ID;
		T m_Payload;
	};
}