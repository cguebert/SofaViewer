#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/MouseManipulator.h>
#include <core/SimpleGUI.h>

#include <render/Scene.h>

#include <sfe/Simulation.h>

#include "GraphImages.h"

#include <chrono>

class SofaDocument : public BaseDocument
{
public:
	SofaDocument(const std::string& type, sfe::Simulation simulation);

	void initUI(simplegui::SimpleGUI& gui) override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(GraphNode* item) override;

protected:
	void parseScene();
	void setupCallbacks();
	void postStep();
	void updateObjects();
	void updateProperties();
	void createGraph();

	void singleStep();
	void resetSimulation();

	void parseNode(GraphNode::SPtr parent, sfe::Node node);
	GraphNode::SPtr createNode(sfe::Object object, GraphNode::SPtr parent);
	GraphNode::SPtr createNode(sfe::Node node, GraphNode::SPtr parent);

	struct SofaModel
	{
		simplerender::Mesh::SPtr mesh;
		simplerender::Material::SPtr material;
		sfe::Object m_sofaObject; // Proxy to the Sofa object in the simulation
		sfe::Data d_vertices, d_normals; // Proxies to access the fields we need in the Sofa object
	};

	SofaModel createSofaModel(sfe::Object& visualModel);

	simplegui::SimpleGUI* m_gui = nullptr;
	simplerender::Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	std::vector<sfe::CallbackHandle> m_sfeCallbacks; // HACK: (TODO) the destruction order relating to the simulation is important
	sfe::Simulation m_simulation;
	GraphImages m_graphImages;
	bool m_updateObjects = false;

	bool m_singleStep = false;
	int m_statusFPS = -1, m_fpsCount = 0;
	std::chrono::high_resolution_clock::time_point m_fpsStart;

	std::vector<SofaModel> m_sofaModels;
	std::vector<simplerender::Mesh::SPtr> m_newMeshes;
	std::vector<simplerender::Material::SPtr> m_newMaterials;

	simplegui::Button::SPtr m_animateButton, m_stepButton, m_resetButton, m_updateGraphButton;
};

inline Graph& SofaDocument::graph()
{ return m_graph; }

//****************************************************************************//

class SofaNode : public GraphNode
{
public:
	using SofaNodePtr = std::shared_ptr<SofaNode>;
	static SofaNodePtr create() { return std::make_shared<SofaNode>(); }

	bool isObject; // if false -> Node
	sfe::Object object; // Cannot put them in an union
	sfe::Node node;		//  as they have a copy constructor
};
