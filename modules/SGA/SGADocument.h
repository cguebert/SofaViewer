#pragma once

#include "MeshDocument.h"

#include <core/SimpleGUI.h>

#include <sga/ObjectFactory.h>

class SGAExecution;

class SGANode : public GraphNode
{
public:
	using SPtr = std::shared_ptr<SGANode>;
	enum class Type { SGA_Root, SGA_Physics, SGA_Collision, SGA_Visual, SGA_Modifier };

	static SPtr create() { return std::make_shared<SGANode>(); }

	Type nodeType;

	sga::ObjectDefinition sgaDefinition; // SGA nodes
};

//****************************************************************************//

struct SimulationProperties
{
	glm::vec3 gravity = { 0, -9.81, 0 };
	double timestep = 0.02;
};

//****************************************************************************//

class SGADocument : public MeshDocument
{
public:
	SGADocument(const std::string& type);

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
	GraphNode::SPtr createNode(const std::string& typeName, const std::string& id); // For the loading of a document

protected:
	void importMesh();
	void convertAndRun();
	void convertAndExport();
	void stopExecution();
	void runClicked();
	
	void addSGANode(GraphNode* parent, sga::ObjectDefinition::ObjectType type);

	void prepareSGAObjectsLists();
	const std::vector<std::string>& SGAObjectsLabels(sga::ObjectDefinition::ObjectType type);
	const std::string& SGAObjectId(sga::ObjectDefinition::ObjectType type, int index);

	void createSGAGraphImages();

	sga::ObjectFactory m_sgaFactory;

	std::vector<std::vector<std::string>> m_sgaObjectsLabels;
	std::vector<std::vector<std::string>> m_sgaObjectsIds;
	std::vector<int> m_graphSGAImages;

	std::shared_ptr<SGAExecution> m_execution;
	SimulationProperties m_simulationProperties;

	simplegui::Button::SPtr m_runButton;
	simplegui::SimpleGUI* m_gui = nullptr;
};

inline Graph& SGADocument::graph()
{ return m_graph; }

inline const std::vector<std::string>& SGADocument::SGAObjectsLabels(sga::ObjectDefinition::ObjectType type)
{ return m_sgaObjectsLabels[static_cast<int>(type)]; }

inline const std::string& SGADocument::SGAObjectId(sga::ObjectDefinition::ObjectType type, int index)
{ return m_sgaObjectsIds[static_cast<int>(type)][index]; }
