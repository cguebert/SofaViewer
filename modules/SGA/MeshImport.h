#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Document;
class Graph;
class ObjectProperties;
class Scene;
class SGANode;

struct aiScene;
struct aiNode;
struct aiMesh;

namespace simplerender
{
class Model;
class Scene;
}

class MeshImport
{
public:
	MeshImport(Document* doc, simplerender::Scene& scene, Graph& graph);
	std::vector<simplerender::Model*> importMeshes(const std::string& filePath); // Import a 3d scene and adds it to the graph, returns the list of the new models
	
private:
	void parseScene(const aiScene* scene);
	void parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, SGANode* parent);
	void parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, SGANode* parent);

	int modelIndex(int meshId);

	Document* m_document;
	simplerender::Scene& m_scene;
	Graph& m_graph;
	std::vector<int> m_modelsIndices; // Mesh id in Assimp scene -> Model id
	std::vector<simplerender::Model*> m_newModels;
};

void populateProperties(SGANode* node, const simplerender::Scene& scene, ObjectProperties* properties);