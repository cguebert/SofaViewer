#pragma once

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <vector>

namespace simplerender
{

class Material
{
public:
	using SPtr = std::shared_ptr<Material>;
	using Color = glm::vec4;

	Color diffuse = { 0.75f, 0.75f, 0.75f, 1.0f };
	Color ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
	Color specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
	float shininess = 45.0f;
	std::string textureFilename;
};

} // namespace simplerender
