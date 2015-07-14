#pragma once

#include <core/Scene.h>
#include <core/MouseManipulator.h>
#include <sfe/Simulation.h>

#include <QString>

class Document
{
public:
	Document();
	bool loadFile(const QString& path);
	void initOpenGL();

	Scene& scene();
	MouseManipulator& mouseManipulator();

protected:
	void parseScene();

	Scene m_scene;
	SofaMouseManipulator m_mouseManipulator;
	sfe::Simulation m_simulation;
};

inline Scene& Document::scene()
{ return m_scene; }

inline MouseManipulator& Document::mouseManipulator()
{ return m_mouseManipulator; }

