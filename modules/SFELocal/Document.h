#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/Scene.h>
#include <core/MouseManipulator.h>
#include <sfe/Simulation.h>

#include "GraphImages.h"

#include <chrono>

class ObjectProperties;

class Document : public BaseDocument
{
public:
	Document(ui::SimpleGUI& gui);
	bool loadFile(const std::string& path) override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(Graph::Node* item) const override;

protected:
	void parseScene();
	void postStep();
	void updateObjects();
	void createGraph();

	void singleStep();

	void parseNode(Graph::NodePtr parent, sfe::Node node);
	Graph::NodePtr createNode(sfe::Object object, Graph::NodePtr parent);
	Graph::NodePtr createNode(sfe::Node node, Graph::NodePtr parent);

	ui::SimpleGUI& m_gui;
	Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sfe::Simulation m_simulation;
	GraphImages m_graphImages;
	bool m_updateObjects = false;

	bool m_singleStep = false;
	int m_statusFPS = -1, m_fpsCount = 0;
	std::chrono::high_resolution_clock::time_point m_fpsStart;
};

class SofaNode : public Graph::Node
{
public:
	using SofaNodePtr = std::shared_ptr<SofaNode>;
	static SofaNodePtr create() { return std::make_shared<SofaNode>(); }

	bool isObject; // if false -> Node
	sfe::Object object; // Cannot put them in an union
	sfe::Node node;		//  as they have a copy constructor
};

inline Graph& Document::graph()
{ return m_graph; }

