#include <render/Model.h>

#define GLEW_STATIC
#include <GL/glew.h>

void Model::init()
{
	mergeIndices();
	prepareBuffers();
	updateIndices();
	updatePositions();
	initShader();
}

void Model::prepareBuffers()
{
	auto	vertSize = m_vertices.size(),
			triSize = m_mergedTriangles.size(),
			texSize = m_texCoords.size();

	if(m_normals.empty())
		m_normals.resize(vertSize);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Vertices
	glGenBuffers(1, &m_verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * vertSize, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normals
	glGenBuffers(1, &m_normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * vertSize, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// Texture coordinates
	if(m_hasTexture && !m_texCoords.empty())
	{
		glGenBuffers(1, &m_texCoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_texCoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * texSize, nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(2);
	}

	// Indices
	glGenBuffers(1, &m_indicesEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triSize * 3 * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

	glBindVertexArray(0); // Unbind the VAO
}

void Model::mergeIndices()
{
	auto tSize = m_triangles.size(), qSize = m_quads.size();
	auto total = tSize + qSize * 2;
	m_mergedTriangles.reserve(total);
	m_mergedTriangles = m_triangles;
	for(auto i = 0u; i < qSize; ++i)
	{
		const auto& quad = m_quads[i];
		m_mergedTriangles.push_back( {quad[0], quad[1], quad[3]} );
		m_mergedTriangles.push_back( {quad[1], quad[2], quad[3]} );
	}
}

void Model::updatePositions()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_verticesVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * m_vertices.size(), &m_vertices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * m_normals.size(), &m_normals[0]);
}

void Model::updateIndices()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesEBO);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_mergedTriangles.size() * 3 * sizeof(GLuint), &m_mergedTriangles[0]);

	if (!m_texCoords.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_texCoordsVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_texCoords.size() * 2 * sizeof(float), &m_texCoords[0]);
	}
}

void Model::initShader()
{

}

void Model::render()
{
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_mergedTriangles.size() * 3, GL_UNSIGNED_INT, nullptr);
}

std::pair<glm::vec3, glm::vec3> Model::boundingBox()
{
	glm::vec3 vMin, vMax;
	for(int i = 0; i < 3; ++i)
	{
		vMin[i] = std::numeric_limits<float>::max();
		vMax[i] = -std::numeric_limits<float>::max();
	}

	for(const auto& vertex : m_vertices)
	{
		for(int i = 0; i < 3; ++i)
		{
			vMin[i] = std::min(vMin[i], vertex[i]);
			vMax[i] = std::max(vMax[i], vertex[i]);
		}
	}

	return std::make_pair(vMin, vMax);
}
