#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <glm/glm.hpp>

namespace Debut
{
	enum class ShaderDataType : uint8_t
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Bool,
		Mat3, Mat4, Struct,
		Sampler2D
	};

	struct ShaderUniform
	{
		std::string Name;
		ShaderDataType Type;
		union UniformData
		{
			float Float;
			int Int;
			bool Bool;

			glm::vec2 Vec2;
			glm::vec3 Vec3;
			glm::vec4 Vec4;

			glm::mat4 Mat4;

			UUID Texture = 0;

			UniformData() {}
		} Data;

		ShaderUniform() {}
		ShaderUniform(const std::string& name, ShaderDataType type, UniformData data) : Name(name), Type(type), Data(data) {}
	};

	inline std::string ShaderDataTypeToString(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Bool: return "Bool";
		case ShaderDataType::Float: return "Float";
		case ShaderDataType::Float2: return "Vec2";
		case ShaderDataType::Float3: return "Vec3";
		case ShaderDataType::Float4: return "Vec4";
		case ShaderDataType::Int: return "Int";
		case ShaderDataType::Mat3: return "Mat3";
		case ShaderDataType::Mat4: return "Mat4";
		case ShaderDataType::Sampler2D: return "Texture";
		}

		return "None";
	}

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;
		virtual std::vector<ShaderUniform> GetUniforms() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* data, uint32_t count) = 0;
		
		virtual void SetFloat(const std::string& name, float uniform) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& uniform) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& uniform) = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& uniform) = 0;

		UUID GetID() { return m_ID; }

		static Ref<Shader> Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
		static Ref<Shader> Create(const std::string& filePath, const std::string& metaFile = "");
	
	protected:
		void CreateOrLoadMeta(const std::string& path);

	protected:
		unsigned int m_ProgramID;
		UUID m_ID;
	};
}