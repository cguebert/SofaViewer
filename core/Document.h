#pragma once

#include <core/Graph.h>
#include <core/Scene.h>
#include <core/MouseManipulator.h>
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
	bool loadFile(const QString& path);
	void initOpenGL();
	void step();

	Scene& scene();
	Graph& graph();
	MouseManipulator& mouseManipulator();

	using ObjectPropertiesPtr = std::shared_ptr<ObjectProperties>;
	ObjectPropertiesPtr objectProperties(size_t id) const;

protected:
	void parseScene();
	void updateObjects();
	void createGraph();

	void parseNode(Graph::NodePtr parent, sfe::Node node);
	Graph::NodePtr createNode(sfe::Object object, Graph::NodePtr parent);
	Graph::NodePtr createNode(sfe::Node node, Graph::NodePtr parent);

	Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sfe::Simulation m_simulation;

	struct ObjectHandle
	{
		bool isObject; // if false -> Node
		sfe::Object object; // Cannot put them in an union
		sfe::Node node;		//  as they have a copy constructor
	};

	using ObjectHandles = std::vector<ObjectHandle>;
	ObjectHandles m_handles;
};

inline Scene& Document::scene()
{ return m_scene; }

inline Graph& Document::graph()
{ return m_graph; }

inline MouseManipulator& Document::mouseManipulator()
{ return m_mouseManipulator; }

