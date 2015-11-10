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

	void init(); // Takes the loaded image and creates an OpenGL texture

	// These two methods open an image and store it in a buffer inside this class. Init must then be called (with a valid OpenGL context)
	bool loadFromFile(const std::string& path);
	bool loadFromMemory(const std::vector<unsigned char>& fileContents);

	void setContents(const std::vector<unsigned char>& contents, int width, int height, bool hasAlpha = false);

	unsigned int id() const;

protected:
	unsigned int m_textureId = 0;

	struct TextureContents
	{
		std::vector<unsigned char> contents;
		int width = 0, height = 0;
		bool hasAlpha = false;
	};
	std::unique_ptr<TextureContents> m_contents;
};

} // namespace simplerender
