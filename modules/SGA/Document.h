#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/MouseManipulator.h>

#include <render/Scene.h>
#include <sfe/Simulation.h>
#include <sga/ObjectFactory.h>
#include <glm/glm.hpp>

struct aiScene;
struct aiNode;
struct aiMesh;

class SGANode : public GraphNode
{
public:
	using SPtr = std::shared_ptr<SGANode>;
	enum class Type { Root, Node, Mesh, Instance };

	static SPtr create() { return std::make_shared<SGANode>(); }

	Type nodeType;

	glm::mat4 transformation;
	simplerender::Scene::ModelPtr model;
	unsigned int meshId;
};

//****************************************************************************//

class Document : public BaseDocument
{
public:
	Document(const std::string& type);

	bool loadFile(const std::string& path) override;
	void initUI(simplegui::SimpleGUI& gui) override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(GraphNode* item) override;
	void graphContextMenu(GraphNode* item, simplegui::Menu& menu) override;

protected:
	SGANode::SPtr createNode(const std::string& name, const std::string& type, SGANode::Type nodeType, GraphNode::SPtr parent);
	void parseScene(const aiScene* scene);
	void parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, GraphNode::SPtr parent);
	void parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, GraphNode::SPtr parent);

	std::shared_ptr<simplerender::Model> createModel(const aiMesh* mesh);
	int modelIndex(int meshId);

	void importMesh();

	void addSGANode(SGANode* parent, sga::ObjectDefinition::ObjectType type);

	simplegui::SimpleGUI* m_gui = nullptr;
	simplerender::Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sga::ObjectFactory m_sgaFactory;

	GraphNode::SPtr m_rootNode;
	size_t m_nextNodeId = 1;
	std::vector<int> m_modelsIndices; // Mesh id in Assimp scene -> Model id
	bool m_reinitScene = false;
};

inline Graph& Document::graph()
{ return m_graph; }
