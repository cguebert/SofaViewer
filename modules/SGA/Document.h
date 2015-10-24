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
	enum class Type { Root, Node, Mesh, Instance, SGA_Root, SGA_Physics, SGA_Collision, SGA_Visual, SGA_Modifier };

	static SPtr create() { return std::make_shared<SGANode>(); }

	Type nodeType;

	glm::mat4 transformation; // Root & Node
	simplerender::Scene::ModelPtr model; // Mesh
	unsigned int meshId; // Instance
	sga::ObjectDefinition sgaDefinition; // SGA nodes
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
	SGANode::SPtr createNode(const std::string& name, const std::string& type, SGANode::Type nodeType, GraphNode* parent);
	void parseScene(const aiScene* scene);
	void parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, GraphNode* parent);
	void parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, GraphNode* parent);

	std::shared_ptr<simplerender::Model> createModel(const aiMesh* mesh);
	int modelIndex(int meshId);

	void importMesh();

	void addSGANode(SGANode* parent, sga::ObjectDefinition::ObjectType type);
	SGANode* childSGANode(SGANode* parent, sga::ObjectDefinition::ObjectType type);

	void prepareSGAObjectsLists();
	const std::string& SGATypeName(sga::ObjectDefinition::ObjectType type);
	const std::vector<std::string>& SGAObjectsLabels(sga::ObjectDefinition::ObjectType type);
	const std::string& SGAObjectId(sga::ObjectDefinition::ObjectType type, int index);
	SGANode::Type SGAToNodeType(sga::ObjectDefinition::ObjectType type);

	simplegui::SimpleGUI* m_gui = nullptr;
	simplerender::Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sga::ObjectFactory m_sgaFactory;

	std::vector<std::string> m_sgaTypesNames;
	std::vector<SGANode::Type> m_sgaToNodeTypes;
	std::vector<std::vector<std::string>> m_sgaObjectsLabels;
	std::vector<std::vector<std::string>> m_sgaObjectsIds;

	GraphNode::SPtr m_rootNode;
	size_t m_nextNodeId = 1;
	std::vector<int> m_modelsIndices; // Mesh id in Assimp scene -> Model id
	bool m_reinitScene = false;
};

inline Graph& Document::graph()
{ return m_graph; }

inline const std::string& Document::SGATypeName(sga::ObjectDefinition::ObjectType type)
{ return m_sgaTypesNames[static_cast<int>(type)]; }

inline const std::vector<std::string>& Document::SGAObjectsLabels(sga::ObjectDefinition::ObjectType type)
{ return m_sgaObjectsLabels[static_cast<int>(type)]; }

inline const std::string& Document::SGAObjectId(sga::ObjectDefinition::ObjectType type, int index)
{ return m_sgaObjectsIds[static_cast<int>(type)][index]; }

inline SGANode::Type Document::SGAToNodeType(sga::ObjectDefinition::ObjectType type)
{ return m_sgaToNodeTypes[static_cast<int>(type)]; }