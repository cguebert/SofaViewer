#pragma once

#include <core/Graph.h>
#include <core/Scene.h>
#include <core/MouseManipulator.h>
#include <core/GraphImages.h>
#include <sfe/Simulation.h>

#include <QString>
#include <functional>

class ViewUpdater
{
public:
	static ViewUpdater& get();

	using UpdateFunc = std::function<void ()>;

	void setSignal(UpdateFunc func);
	void update();

protected:
	UpdateFunc m_func;
};

class ObjectProperties;

class Document
{
public:
	Document();
	bool loadFile(const std::string& path);

	void initOpenGL();
	void resize(int width, int height);
	void render();

	void step();
	void animate();

	Scene& scene();
	Graph& graph();
	MouseManipulator& mouseManipulator();

	using ObjectPropertiesPtr = std::shared_ptr<ObjectProperties>;
	ObjectPropertiesPtr objectProperties(Graph::Node* item) const;

protected:
	void parseScene();
	void postStep();
	void updateObjects();
	void createGraph();

	void parseNode(Graph::NodePtr parent, sfe::Node node);
	Graph::NodePtr createNode(sfe::Object object, Graph::NodePtr parent);
	Graph::NodePtr createNode(sfe::Node node, Graph::NodePtr parent);

	Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sfe::Simulation m_simulation;
	GraphImages m_graphImages;
	bool m_updateObjects = false;
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

inline Scene& Document::scene()
{ return m_scene; }

inline Graph& Document::graph()
{ return m_graph; }

inline MouseManipulator& Document::mouseManipulator()
{ return m_mouseManipulator; }

