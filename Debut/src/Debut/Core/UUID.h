#pragma once
#include <xhash>

namespace Debut
{
	// TODO: have a look into how safe it is to randomly generate a number and trust that it won't cause a collision
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<Debut::UUID>
	{
		std::size_t operator()(const Debut::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
