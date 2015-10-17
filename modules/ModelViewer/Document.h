#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/MouseManipulator.h>

#include <render/Scene.h>
#include <sfe/Simulation.h>
#include <glm/glm.hpp>

struct aiScene;
struct aiNode;
struct aiMesh;
class ModelNode;

class ModelNode : public GraphNode
{
public:
	using SPtr = std::shared_ptr<ModelNode>;
	enum class Type { Node, Mesh, Instance };

	static SPtr create() { return std::make_shared<ModelNode>(); }

	Type nodeType;

	glm::mat4 transformation;
	Scene::ModelPtr model;
	unsigned int meshId;
};

//****************************************************************************//

class Document : public BaseDocument
{
public:
	Document(ui::SimpleGUI& gui);
	std::string documentType() override;

	bool loadFile(const std::string& path) override;
	void initUI() override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(GraphNode* item) override;

protected:
	ModelNode::SPtr createNode(const std::string& name, const std::string& type, ModelNode::Type nodeType, GraphNode::SPtr parent);
	void parseScene(const aiScene* scene);
	void parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, GraphNode::SPtr parent);
	void parseMeshInstance(const aiScene* scene, unsigned int id, GraphNode::SPtr parent);

	std::shared_ptr<Model> createModel(const aiMesh* mesh);
	int modelIndex(int meshId);

	ui::SimpleGUI& m_gui;
	Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;

	GraphNode::SPtr m_rootNode;
	size_t m_nextNodeId = 1;
	std::vector<int> m_modelsIndices; // Mesh id in Assimp scene -> Model id
};

inline Graph& Document::graph()
{ return m_graph; }
