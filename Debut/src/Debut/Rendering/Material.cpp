#include <Debut/dbtpch.h>
#include <Debut/Rendering/Material.h>

#define FIND_UNIFORM(name)	if (m_Uniforms.find(##name) == m_Uniforms.end()) \
							{	\
								Log.CoreError("Uniform {0} is not supported by the material {1}", ##name, m_Name);	\
								return;	\
							}

#define CHECK_TYPE(name, type)  ShaderUniform uniform = m_Uniforms[##name];	\
								if (uniform.Type != ShaderDataType::Float)	\
								{	\
									Log.CoreWarn("The uniform {0} isn't of type Float. Try using the correct Set function.", ##name);	\
									return;	\
								}

namespace Debut
{
	void Material::SetFloat(const std::string& name, float val)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float);
		m_Uniforms[name].Data.Float = val;
	}

	void Material::SetVec2(const std::string& name, const glm::vec2& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float2);
		m_Uniforms[name].Data.Vec2 = vec;
	}

	void Material::SetVec3(const std::string& name, const glm::vec3& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float3);
		m_Uniforms[name].Data.Vec3 = vec;
	}

	void Material::SetVec4(const std::string& name, const glm::vec4& vec)
	{
		FIND_UNIFORM(name);
		CHECK_TYPE(name, ShaderDataType::Float4);
		m_Uniforms[name].Data.Vec4 = vec;
	}

	void Material::SetInt(const std::string& name, int val)
	{

	}

	void Material::SetTexture(const Ref<Texture2D> texture)
	{

	}
}