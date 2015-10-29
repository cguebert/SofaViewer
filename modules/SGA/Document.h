#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/MouseManipulator.h>

#include <render/Scene.h>
#include <sfe/Simulation.h>
#include <sga/ObjectFactory.h>

class SGAExecution;

class SGANode : public GraphNode
{
public:
	using SPtr = std::shared_ptr<SGANode>;
	enum class Type { Root, Node, Mesh, Instance, SGA_Root, SGA_Physics, SGA_Collision, SGA_Visual, SGA_Modifier };

	static SPtr create() { return std::make_shared<SGANode>(); }

	Type nodeType;

	glm::mat4 transformation; // Root & Node
	simplerender::Scene::ModelPtr model; // Mesh
	int meshId = -1; // Instance
	sga::ObjectDefinition sgaDefinition; // SGA nodes
};

//****************************************************************************//

class Document : public BaseDocument
{
public:
	Document(const std::string& type);

	bool loadFile(const std::string& path) override;
	bool saveFile(const std::string& path) override;
	void initUI(simplegui::SimpleGUI& gui) override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(GraphNode* item) override;
	void closeObjectProperties(GraphNode* item, ObjectPropertiesPtr ptr, bool accepted) override;
	void graphContextMenu(GraphNode* item, simplegui::Menu& menu) override;

	SGANode::SPtr createNode(const std::string& name, SGANode::Type nodeType, GraphNode* parent, int position = -1);

protected:
	void importMesh();
	void convertAndRun();
	
	void addSGANode(GraphNode* parent, sga::ObjectDefinition::ObjectType type);

	void prepareSGAObjectsLists();
	const std::vector<std::string>& SGAObjectsLabels(sga::ObjectDefinition::ObjectType type);
	const std::string& SGAObjectId(sga::ObjectDefinition::ObjectType type, int index);

	void createGraphImages();

	void updateInstances();
	void updateInstances(SGANode* item, const glm::mat4& transformation);

	simplegui::SimpleGUI* m_gui = nullptr;
	simplerender::Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sga::ObjectFactory m_sgaFactory;

	std::vector<std::vector<std::string>> m_sgaObjectsLabels;
	std::vector<std::vector<std::string>> m_sgaObjectsIds;

	SGANode::SPtr m_rootNode;
	size_t m_nextNodeId = 1;
	std::vector<int> m_graphImages;
	std::vector<simplerender::Model*> m_newModels;

	std::shared_ptr<SGAExecution> m_execution;
};

inline Graph& Document::graph()
{ return m_graph; }

inline const std::vector<std::string>& Document::SGAObjectsLabels(sga::ObjectDefinition::ObjectType type)
{ return m_sgaObjectsLabels[static_cast<int>(type)]; }

inline const std::string& Document::SGAObjectId(sga::ObjectDefinition::ObjectType type, int index)
{ return m_sgaObjectsIds[static_cast<int>(type)][index]; }
