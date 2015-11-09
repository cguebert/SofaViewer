#pragma once

#include <core/Property.h>

#include <functional>
#include <string>
#include <vector>

class GraphNode;
class ObjectProperties;

namespace simplegui
{

using CallbackFunc = std::function<void()>;
using BoolCallbackFunc = std::function<void(bool)>;

class CORE_API Panel
{
public:
	virtual ~Panel();

	virtual void addButton(const std::string& name, const std::string& help,
						   CallbackFunc callback,
						   int row = -1, int column = 0,
						   int rowSpan = 1, int columnSpan = 1) = 0;

	virtual void addProperty(Property::SPtr property,
							 int row = -1, int column = 0,
							 int rowSpan = 1, int columnSpan = 1) = 0;
};

/******************************************************************************/

class CORE_API Dialog
{
public:
	using SPtr = std::shared_ptr<Dialog>;
	virtual ~Dialog();

	virtual Panel& content() = 0; // Add buttons and property widgets
	virtual void setMinimumSize(int width, int height) = 0; // If you want to enlarge the default size
	virtual bool exec() = 0; // Modal dialog (blocks, then returns true if "Ok" was pressed)
	virtual void show(BoolCallbackFunc finishedCallback) = 0; // Modeless dialog (doesn't block). The callback is triggered when the user closes the dialog (true if clicked on Ok)
};

/******************************************************************************/

class CORE_API Menu
{
public:
	virtual ~Menu();

	virtual void addItem(const std::string& name, const std::string& help, CallbackFunc callback) = 0;
	virtual Menu& addMenu(const std::string& name) = 0;
	virtual void addSeparator() = 0;
};

/******************************************************************************/

class CORE_API Settings
{
public:
	virtual ~Settings();

	virtual void set(const std::string& name, int val) = 0;
	virtual void set(const std::string& name, double val) = 0;
	virtual void set(const std::string& name, const std::string& val) = 0;
	virtual void set(const std::string& name, const std::vector<int>& val) = 0;
	virtual void set(const std::string& name, const std::vector<double>& val) = 0;
	virtual void set(const std::string& name, const std::vector<std::string>& val) = 0;

	virtual bool get(const std::string& name, int& val) = 0;
	virtual bool get(const std::string& name, double& val) = 0;
	virtual bool get(const std::string& name, std::string& val) = 0;
	virtual bool get(const std::string& name, std::vector<int>& val) = 0;
	virtual bool get(const std::string& name, std::vector<double>& val) = 0;
	virtual bool get(const std::string& name, std::vector<std::string>& val) = 0;
};

/******************************************************************************/

class CORE_API SimpleGUI
{
public:
	enum class MenuType : unsigned char
	{ File, Tools, View, Help };

	using ObjectPropertiesPair = std::pair<GraphNode*, std::shared_ptr<ObjectProperties>>;

	virtual ~SimpleGUI();

	virtual Menu& getMenu(MenuType type) = 0;

	virtual Panel& buttonsPanel() = 0; // Access to the buttons dock (on the side of the OpenGL view)

	virtual int addStatusBarZone(const std::string& text) = 0; // Use the text parameter to set the minimum size of the zone
	virtual void setStatusBarText(int id, const std::string& text) = 0;

	virtual Dialog::SPtr createDialog(const std::string& title) = 0;
	virtual std::string getOpenFileName(const std::string& caption, const std::string& path, const std::string& filters) = 0; // Returns an empty string if the user cancels
	virtual std::string getSaveFileName(const std::string& caption, const std::string& path, const std::string& filters) = 0;

	virtual void updateView() = 0; // Update the OpenGL view

	virtual void closePropertiesDialog(GraphNode* node) = 0;
	virtual void closeAllPropertiesDialogs() = 0;
	virtual std::vector<ObjectPropertiesPair> getOpenedPropertiesDialogs() = 0;

	virtual Settings& settings() = 0;
};

} // namespace simplegui
