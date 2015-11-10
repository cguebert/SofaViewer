#include <render/Material.h>
#include <render/Texture.h>

namespace simplerender
{

void Material::init()
{
	if (!textureFilename.empty())
	{
		m_texture = std::make_shared<Texture>();
		m_texture->load(textureFilename);
	}		
}

unsigned int Material::textureId() const
{
	if (m_texture)
		return m_texture->id();
	else
		return 0;
}

} // namespace simplerender
