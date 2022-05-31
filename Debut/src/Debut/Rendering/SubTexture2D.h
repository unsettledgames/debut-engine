#pragma once
#include <glm/glm.hpp>
#include <Debut/Rendering/Texture.h>

namespace Debut
{
	class SubTexture2D
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		const Ref<Texture2D> GetTexture() const { return m_Texture; }
		const glm::vec2* GetTexCoords() const { return m_TexCoords; }

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& spriteSizeInTiles, const glm::vec2& tileSize);

	private:
		Ref<Texture2D> m_Texture;

		glm::vec2 m_TexCoords[4];
	};

}

