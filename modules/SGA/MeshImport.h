#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <core/PropertiesUtils.h>

class Graph;
class ObjectProperties;
class MeshDocument;
class MeshNode;
class Scene;

struct aiScene;
struct aiNode;
struct aiMesh;

namespace simplerender
{
class Material;
class Mesh;
class Scene;
}

namespace property
{
	namespace details
	{

		template <>
		struct ArrayTraits<glm::vec2>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 2;
			using value_type = glm::vec2::value_type;
		};

		template <>
		struct ArrayTraits<glm::vec3>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 3;
			using value_type = glm::vec3::value_type;
		};

		template <>
		struct ArrayTraits<glm::vec4>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 4;
			using value_type = glm::vec4::value_type;
		};

		template <>
		struct ArrayTraits<glm::mat4>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 4;
			using value_type = glm::mat4::col_type;
		};
	}
}

class MeshImport
{
public:
	using Meshes = std::vector<simplerender::Mesh*>;
	using Materials = std::vector<simplerender::Material*>;

	MeshImport(MeshDocument* doc, simplerender::Scene& scene, Graph& graph);
	std::pair<Meshes, Materials> importMeshes(const std::string& filePath); // Import a 3d scene and adds it to the graph, returns the lists of the new meshes and new materials

	static void findTextures(Materials& materials, const std::string& modelPath);
	
private:
	void parseScene(const aiScene* scene);
	void parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, MeshNode* parent);
	void parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, MeshNode* parent);

	void addMeshes(const aiScene* scene);
	void addMaterials(const aiScene* scene);

	int meshIndex(int meshId);
	int materialIndex(int materialId);

	MeshDocument* m_document;
	simplerender::Scene& m_scene;
	Graph& m_graph;
	std::vector<std::pair<int, int>> m_meshesIndices, m_materialIndices; // Id in Assimp scene -> Id in our scene
	Meshes m_newMeshes;
	Materials m_newMaterials;
};
