#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/MouseManipulator.h>
#include <render/Scene.h>
#include <sfe/Simulation.h>

struct aiScene;

class Document : public BaseDocument
{
public:
	Document(ui::SimpleGUI& gui);
	std::string documentType() override;

	bool loadFile(const std::string& path) override;
	void initUI() override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(GraphNode* item) override;

protected:
	void parseScene(const aiScene* scene);

	ui::SimpleGUI& m_gui;
	Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;

	GraphNode::Ptr m_rootNode;
};

inline Graph& Document::graph()
{ return m_graph; }

//****************************************************************************//

class ModelNode : public GraphNode
{
public:
	using ModelNodePtr = std::shared_ptr<ModelNode>;
	static ModelNodePtr create() { return std::make_shared<ModelNode>(); }

};
