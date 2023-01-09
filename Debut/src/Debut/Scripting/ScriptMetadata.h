#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>

namespace Debut
{
	class ScriptMetadata
	{
	public:
		ScriptMetadata(const std::string& filePath, const std::string& metaPath);

		inline UUID GetID() { return m_Script; }
		inline std::string GetName() { return m_ClassName; }

	private:
		UUID m_Script;
		std::string m_ClassName = "None";
	};
}
