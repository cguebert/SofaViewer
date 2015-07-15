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

protected:
	void parseScene();
	void updateObjects();
	void createGraph();

	Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;
	sfe::Simulation m_simulation;
};

inline Scene& Document::scene()
{ return m_scene; }

inline Graph& Document::graph()
{ return m_graph; }

inline MouseManipulator& Document::mouseManipulator()
{ return m_mouseManipulator; }

