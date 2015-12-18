#pragma once

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <vector>

namespace simplerender
{

class Texture;

enum class TextureType
{
	Diffuse, Specular, Bump, Normal
};

const std::vector<std::string>& getTexturesTypes(); // Return the list of names of the texture types
const std::string& getName(TextureType type); // Return the corresponding name of the given type
TextureType getTextureType(const std::string& name); // Return the corresponding type (Diffuse if the name is incorrect)

struct TextureData
{
	TextureData(TextureType type, std::string filePath) : type(type), filePath(filePath) {}

	TextureType type;
	std::string filePath;
	std::shared_ptr<Texture> texture;
};

class Material
{
public:
	using SPtr = std::shared_ptr<Material>;
	using Color = glm::vec4;

	void init(); // Mainly to load the texture
	unsigned int textureId(TextureType type, int id) const;

	Color diffuse = { 0.75f, 0.75f, 0.75f, 1.0f };
	Color ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
	Color specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
	float shininess = 45.0f;
	std::vector<TextureData> textures;
};

} // namespace simplerender
