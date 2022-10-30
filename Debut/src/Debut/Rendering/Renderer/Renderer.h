#pragma once

#include "RendererAPI.h"

namespace Debut
{
	struct RendererConfig
	{
		bool RenderSurfaces = true;
		bool RenderWireframe = false;
		bool RenderColliders = false;

		enum class RenderingMode
		{
			Standard = 0, Untextured = 1, Depth = 2, None
		};
		RenderingMode RenderingMode = RenderingMode::Standard;

		bool operator==(const RendererConfig& a) const
		{
			return a.RenderSurfaces == RenderSurfaces && a.RenderWireframe == RenderWireframe &&
				a.RenderColliders == RenderColliders && a.RenderingMode == RenderingMode;
		}

		bool operator!=(const RendererConfig& a) const
		{
			return !(a.RenderSurfaces == RenderSurfaces && a.RenderWireframe == RenderWireframe &&
				a.RenderColliders == RenderColliders && a.RenderingMode == RenderingMode);
		}

		RendererConfig() = default;
		RendererConfig(const RendererConfig& config) = default;
	};

	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResized(uint32_t width, uint32_t height);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		inline static void SetApi(RendererAPI::API val) { RendererAPI::SetAPI(val); }

		inline static RendererConfig GetConfig() { return m_Config; }
		inline static void SetConfig(const RendererConfig& config) { m_Config = config; }
	
	private:
		static RendererConfig m_Config;
	};
}