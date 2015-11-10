#pragma once

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <vector>

namespace simplerender
{

class Texture;

class Material
{
public:
	using SPtr = std::shared_ptr<Material>;
	using Color = glm::vec4;

	void init(); // Mainly to load the texture
	unsigned int textureId() const;

	Color diffuse = { 0.75f, 0.75f, 0.75f, 1.0f };
	Color ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
	Color specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
	float shininess = 45.0f;
	std::string textureFilename;

private:
	std::shared_ptr<Texture> m_texture;
};

} // namespace simplerender
