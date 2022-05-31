#pragma once

#include <Debut/Core/Log.h>
#include <string>
#include <unordered_map>

namespace Debut
{
	template <typename Key, typename Value>
	class AssetCache
	{
	public:
		AssetCache() = default;
		AssetCache(const std::string& name) : m_Name(name) {}
		AssetCache(uint32_t size, const std::string& name) : m_Name(name)
		{
			m_Cache.reserve(size);
		}

		inline Value Get(Key id)
		{
			if (m_Cache.find(id) != m_Cache.end())
				return m_Cache[id];

			Log.CoreWarn("Couldn't find the resource with id {0} in the {1} cache", id, m_Name);
			return {};
		}

		inline void Put(Key id, Value item)
		{
			if (m_Cache.find(id) != m_Cache.end())
			{
				Log.CoreWarn("Attempted to add asset with ID already taken (skipping)");
				return;
			}

			m_Cache[id] = item;
		}

		inline Value Remove(Key id)
		{
			if (m_Cache.find(id) != m_Cache.end())
			{
				Value ret = m_Cache[id];
				m_Cache.erase(ret);

				return ret;
			}
		}

		inline bool Has(Key id)
		{
			if (m_Cache.find(id) != m_Cache.end())
				return true;
			return false;
		}

		Value operator[](const std::string& id)
		{
			return Get(id);
		}

		void SetName(const std::string& name) { m_Name = name; }
		void Resize(uint32_t size) { m_Cache.reserve(size); }

	private:
		std::unordered_map<Key, Value> m_Cache;
		std::string m_Name;
	};
}