#pragma once

#include <render/Shader.h>

#include <glm/glm.hpp>

#include <array>
#include <vector>

namespace simplerender
{

using Vertices = std::vector < glm::vec3 >;
using Normals = std::vector < glm::vec3 >;
using Triangle = std::array < unsigned int, 3 >;
using Triangles = std::vector < Triangle >;
using Quad = std::array < unsigned int, 4 >;
using Quads = std::vector < Quad >;
using TexCoords = std::vector < glm::vec2 >;

using IdList = std::vector < int >;

class Model
{
public:
	void init();
	void prepareBuffers();
	void updatePositions();
	void updateIndices();
	void mergeIndices();
	void initShader();
	void render();

	Vertices m_vertices;
	Normals m_normals;

	Triangles m_triangles;
	Quads m_quads;

	TexCoords m_texCoords;
	unsigned int  m_texture;
	bool m_hasTexture = false;

	glm::vec4 m_color = { 1, 1, 1, 1 };

	Triangles m_mergedTriangles; // With the quads
	unsigned int  m_VAO, m_verticesVBO, m_normalsVBO, m_texCoordsVBO, m_indicesEBO;

	ShaderProgram m_program;
};

std::pair<glm::vec3, glm::vec3> boundingBox(const Model& model);
std::pair<glm::vec3, glm::vec3> boundingBox(const Model& model, const glm::mat4& transformation);

} // namespace simplerender
