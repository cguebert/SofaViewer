#pragma once

#include <render/Shader.h>

#include <glm/glm.hpp>

#include <array>
#include <vector>

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

	std::pair<glm::vec3, glm::vec3> boundingBox();

	Vertices m_vertices;
	Normals m_normals;

	Triangles m_triangles;
	Quads m_quads;

	TexCoords m_texCoords;
	unsigned int  m_texture;
	bool m_hasTexture = false;

	glm::vec4 m_color;

	Triangles m_mergedTriangles; // With the quads
	unsigned int  m_VAO, m_verticesVBO, m_normalsVBO, m_texCoordsVBO, m_indicesEBO;

	ShaderProgram m_program;
};
