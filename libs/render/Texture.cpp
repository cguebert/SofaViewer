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

bool Texture::load(const std::string& path)
{
	int width, height, comp;
	stbi_uc* image = stbi_load(path.c_str(), &width, &height, &comp, 3);
	if (!image)
		return false;

	glGenTextures(1, &m_textureId);
	glBindTexture(GL_TEXTURE_2D, m_textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

unsigned int Texture::id() const
{
	return m_textureId;
}

} // namespace simplerender
