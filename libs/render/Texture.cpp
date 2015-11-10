#include <render/Texture.h>

#define GLEW_STATIC
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace simplerender
{

Texture::~Texture()
{
	glDeleteTextures(1, &m_textureId);
}

bool Texture::loadFromFile(const std::string& path)
{
	int width, height, comp;
	stbi_uc* image = stbi_load(path.c_str(), &width, &height, &comp, 3);
	if (!image)
		return false;

	std::vector<unsigned char> contents(image, image + (width * height * 3));
	stbi_image_free(image);
	setContents(contents, width, height);
	return true;
}

bool Texture::loadFromMemory(const std::vector<unsigned char>& fileContents)
{
	int width, height, comp;
	stbi_uc* image = stbi_load_from_memory(&fileContents[0], fileContents.size(), &width, &height, &comp, 3);
	if (!image)
		return false;

	std::vector<unsigned char> contents(image, image + (width * height * 3));
	stbi_image_free(image);
	setContents(contents, width, height);
	return true;
}

void Texture::setContents(const std::vector<unsigned char>& contents, int width, int height, bool hasAlpha)
{
	m_contents = std::make_unique<TextureContents>();
	m_contents->contents = contents;
	m_contents->width = width;
	m_contents->height = height;
	m_contents->hasAlpha = hasAlpha;
}

void Texture::init()
{
	if (m_contents)
	{
		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
			m_contents->width, m_contents->height, 0, 
			m_contents->hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, &m_contents->contents[0]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	m_contents.reset();
}

unsigned int Texture::id() const
{
	return m_textureId;
}

} // namespace simplerender
