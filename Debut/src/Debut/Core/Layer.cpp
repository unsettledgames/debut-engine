#include "Debut/dbtpch.h"

#include "Layer.h"

namespace Debut
{
	Layer::Layer(const std::string& name)
	{
		m_DebugName = name;
	}

	Layer::~Layer() {}
}