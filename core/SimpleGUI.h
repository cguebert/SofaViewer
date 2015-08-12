#pragma once

#include <core/Property.h>

#include <functional>
#include <string>

namespace ui
{

using CallbackFunc = std::function<void()>;

class Panel
{
public:
	virtual ~Panel() {}

	virtual void addButton(const std::string& name, const std::string& help,
						   CallbackFunc callback,
						   int row = -1, int column = 0,
						   int rowSpan = 1, int columnSpan = 1) = 0;

	virtual void addProperty(Property::PropertyPtr property,
							 int row = -1, int column = 0,
							 int rowSpan = 1, int columnSpan = 1) = 0;
};

/******************************************************************************/

class Dialog
{
public:
	virtual ~Dialog() {}

	virtual Panel& content() = 0; // Add buttons and property widgets
	virtual bool exec() = 0; // Modal dialog (blocks, then returns true if "Ok" was pressed)
	virtual void show() = 0; // Modeless dialog (doesn't block)
};

/******************************************************************************/

class SimpleGUI
{
public:
	virtual ~SimpleGUI() {}

	enum class Menu : unsigned char
	{ File, Tools, View, Help };

	virtual void addMenuItem(Menu menu, const std::string& name, const std::string& help, CallbackFunc callback) = 0;

	virtual Panel& buttonsPanel() = 0; // Access to the buttons dock (on the side of the OpenGL view)

	virtual int addStatusBarZone(const std::string& text) = 0; // Use the text parameter to set the minimum size of the zone
	virtual void setStatusBarText(int id, const std::string& text) = 0;

	using DialogPtr = std::shared_ptr<Dialog>;
	virtual DialogPtr createDialog(const std::string& title) = 0;

	virtual void updateView() = 0; // Update the OpenGL view
};

} // namespace ui
