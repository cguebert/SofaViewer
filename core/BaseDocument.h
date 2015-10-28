#pragma once

#include <core/core.h>
#include <memory>
#include <string>

class Graph;
class GraphNode;
class ObjectProperties;
struct MouseEvent;

namespace simplegui
{
class Menu;
class SimpleGUI; 
}

class CORE_API BaseDocument
{
public:
	BaseDocument(const std::string& type);
	virtual ~BaseDocument() {}

	std::string documentType() const; // Returns the name of the document, as given in RegisterDocument
	std::string module() const; // Returns the parent module of the document
	std::string modulePath() const; // Returns the emplacement of the data files for this module

	virtual bool loadFile(const std::string& /*path*/) { return false; }
	virtual bool saveFile(const std::string& /*path*/) { return false; }
	virtual void initUI(simplegui::SimpleGUI& /*gui*/) = 0; // The document is now tied to the GUI, the implemenation can create the graph and the menus

	virtual void initOpenGL() {}
	virtual void resize(int /*width*/, int /*height*/) {}
	virtual void render() {}

	virtual bool mouseEvent(const MouseEvent& /*event*/) { return false; } // Return true if an view update is necessary

	virtual Graph& graph() = 0;

	using ObjectPropertiesPtr = std::shared_ptr<ObjectProperties>;
	virtual ObjectPropertiesPtr objectProperties(GraphNode* /*item*/) { return nullptr; } // Ask the object properties of a graph node
	virtual void closeObjectProperties(GraphNode* /*item*/, ObjectPropertiesPtr /*ptr*/, bool /*accepted*/) {} // Signal that these object properties are not used anymore (closed dialog)

	virtual void graphContextMenu(GraphNode* /*item*/, simplegui::Menu& /*menu*/) {}; // The context menu when right clicking a graph node (if empty, will not be shown)

private:
	std::string m_documentType;
};
