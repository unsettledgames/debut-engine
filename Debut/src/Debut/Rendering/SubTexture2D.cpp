#include "Debut/dbtpch.h"
#include "SubTexture2D.h"

#include <Debut/Rendering/Texture.h>

namespace Debut
{
	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& spriteSizeInTiles, const glm::vec2& tileSize)
	{
		uint32_t sheetWidth = texture->GetWidth();
		uint32_t sheetHeight = texture->GetHeight();
		
		glm::vec2 min = glm::vec2((float)(coords.x * tileSize.x) / sheetWidth, (float)(coords.y * tileSize.y) / sheetHeight);
		glm::vec2 max = glm::vec2((float)((coords.x + spriteSizeInTiles.x) * tileSize.x) / sheetWidth, (float)((coords.y + spriteSizeInTiles.y) * tileSize.y) / sheetHeight);

		return CreateRef<SubTexture2D>(texture, min, max);
	}

	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max) : m_Texture(texture)
	{
		m_TexCoords[0] = glm::vec2(min.x, min.y);
		m_TexCoords[1] = glm::vec2(max.x, min.y);
		m_TexCoords[2] = glm::vec2(max.x, max.y);
		m_TexCoords[3] = glm::vec2(min.x, max.y);
	}
}
