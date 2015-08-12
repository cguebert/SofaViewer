#pragma once

#include <core/Graph.h>
#include <core/MouseManipulator.h>

#include <functional>

class ObjectProperties;

namespace ui { class SimpleGUI; }

class BaseDocument
{
public:
	BaseDocument(ui::SimpleGUI& /*gui*/) {}
	virtual ~BaseDocument() {}

	virtual bool loadFile(const std::string& /*path*/) { return false; }
	virtual void initUI() = 0; // The document is now tied to the GUI, the implemenation can create the graph and the menus

	virtual void initOpenGL() {}
	virtual void resize(int /*width*/, int /*height*/) {}
	virtual void render() {}

	virtual bool mouseEvent(const MouseEvent& /*event*/) { return false; } // Return true if an view update is necessary

	virtual Graph& graph() = 0;

	using ObjectPropertiesPtr = std::shared_ptr<ObjectProperties>;
	virtual ObjectPropertiesPtr objectProperties(Graph::Node* /*item*/) { return nullptr; } // Ask the object properties of a graph node
	virtual void closeObjectProperties(ObjectPropertiesPtr ptr) {} // Signal that these object properties are not used anymore (closed dialog)
};
