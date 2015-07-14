#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>

#include <glm/glm.hpp>
#include <sfe/DataTypeTrait.h>
#include <vector>

#include <sfe/Object.h>

using Vertices = std::vector < glm::vec3 >;
using Normals = std::vector < glm::vec3 >;
using Triangle = std::array < GLuint, 3 >;
using Triangles = std::vector < Triangle >;
using Quad = std::array < GLuint, 4 >;
using Quads = std::vector < Quad >;
using TexCoords = std::vector < glm::vec2 >;

using IdList = std::vector < int >;

namespace sfe
{
	template<> struct DataTypeTrait<glm::vec2> : public ArrayTypeTrait<glm::vec2, 2>{};
	template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

class Model : public QOpenGLFunctions_3_3_Core
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
	GLuint m_texture;
	bool m_hasTexture = false;

	glm::vec4 m_color;

	sfe::Object m_sofaObject; // Proxy to the Sofa object in the simulation
	sfe::Data d_vertices, d_normal; // Proxies to access the fields we need in the Sofa object

	Triangles m_mergedTriangles; // With the quads
	GLuint m_VAO, m_verticesVBO, m_normalsVBO, m_texCoordsVBO, m_indicesEBO;

	QOpenGLShaderProgram m_program;
};
