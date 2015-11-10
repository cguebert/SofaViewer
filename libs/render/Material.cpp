#include <render/Material.h>
#include <render/Texture.h>

namespace simplerender
{

void Material::init()
{
	if (!textureFilename.empty())
	{
		texture = std::make_shared<Texture>();
		if (!texture->loadFromFile(textureFilename))
			texture.reset();
	}		

	if (texture)
		texture->init();
}

unsigned int Material::textureId() const
{
	if (texture)
		return texture->id();
	else
		return 0;
}

} // namespace simplerender
