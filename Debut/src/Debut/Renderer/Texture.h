#pragma once

#include "Debut/Core/Log.h"
#include "Debut/Core/UUID.h"
#include "Debut/Core/Core.h"

namespace Debut
{
	enum class Texture2DParameter
	{
		FILTERING_POINT = 0, FILTERING_LINEAR,
		WRAP_REPEAT, WRAP_CLAMP, NONE
	};

	struct Texture2DConfig
	{
		Texture2DParameter Filtering;
		Texture2DParameter WrapMode;
		UUID ID;
	};

	static std::string Tex2DParamToString(Texture2DParameter parameter)
	{
		switch (parameter)
		{
		case Texture2DParameter::FILTERING_LINEAR: return "Linear";
		case Texture2DParameter::FILTERING_POINT: return "Point";
		case Texture2DParameter::WRAP_CLAMP: return "Clamp";
		case Texture2DParameter::WRAP_REPEAT: return "Repeat";
		}

		Log.AppWarn("Texture parameter {0} not supported", (uint32_t)parameter);
		return "";
	}
	static Texture2DParameter StringToTex2DParam(const std::string& param)
	{
		if (param == "Linear") return Texture2DParameter::FILTERING_LINEAR;
		if (param == "Point") return Texture2DParameter::FILTERING_POINT;
		if (param == "Clamp") return Texture2DParameter::WRAP_CLAMP;
		if (param == "Repeat") return Texture2DParameter::WRAP_REPEAT;

		Log.AppWarn("Texture parameter {0} not supported", param);
		return Texture2DParameter::NONE;
	}

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual std::string GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool operator==(const Texture& other) const = 0;
		
	};

	class Texture2D : public Texture
	{
	protected:
		// Texture parameters
		Texture2DParameter m_FilteringMode = Texture2DParameter::FILTERING_LINEAR;
		Texture2DParameter m_WrapMode = Texture2DParameter::WRAP_CLAMP;

		UUID m_ID;

	public:
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

		Texture2DParameter GetFilteringMode() { return m_FilteringMode; }
		Texture2DParameter GetWrapMode() { return m_WrapMode; }

		void SetFilteringMode(Texture2DParameter param) { m_FilteringMode = param; }
		void SetWrapMode(Texture2DParameter param) { m_WrapMode = param; }
		
		UUID GetID() { return m_ID; }
		float GetAspectRatio() { return (float)GetWidth() / GetHeight(); }

		virtual void Reload() = 0;
	};
}
