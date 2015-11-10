#pragma once

#include <memory>
#include <string>
#include <vector>

namespace simplerender
{

// To destroy the texture object only when every copy is destroyed
class TextureId;

class Texture
{
public:
	~Texture();
	bool load(const std::string& path);

	unsigned int id() const;

protected:

	std::string m_texturePath;
	unsigned int m_textureId = 0;
};

} // namespace simplerender
