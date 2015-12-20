#include <render/Material.h>
#include <render/Texture.h>

namespace simplerender
{

const std::vector<std::string>& getTexturesTypes()
{
	static std::vector<std::string> names = { "Diffuse", "Specular", "Bump", "Normal" };
	return names;
}

const std::string& getName(TextureType type)
{
	return getTexturesTypes()[static_cast<int>(type)];
}

TextureType getTextureType(const std::string& name)
{
	const auto names = getTexturesTypes();
	auto it = std::find(names.begin(), names.end(), name);
	if (it == names.end())
		return TextureType::Diffuse;

	return static_cast<TextureType>(std::distance(names.begin(), it));
}

void Material::init()
{
	for (auto& tex : textures)
	{
		auto& texture = tex.texture;
		if (!tex.filePath.empty())
		{
			texture = std::make_shared<Texture>();
			if (!texture->loadFromFile(tex.filePath))
				texture.reset();
		}

		if (texture)
			texture->init();
	}
}

unsigned int Material::textureId(TextureType type, int id) const
{
	int i = 0;
	auto typeVal = static_cast<unsigned int>(type);
	for (auto& tex : textures)
	{
		if (tex.type == typeVal)
		{
			if (i == id)
				return tex.texture ? tex.texture->id() : 0;
			++i;
		}
	}

	return 0;
}

} // namespace simplerender
