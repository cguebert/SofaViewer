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

class CORE_API Button
{
public:
	using SPtr = std::shared_ptr<Button>;
	virtual ~Button();

	virtual std::string title() const = 0;
	virtual void setTitle(const std::string& title) = 0;

	virtual std::string help() const = 0;
	virtual void setHelp(const std::string& help) = 0;

	virtual bool checkable() const = 0;
	virtual void setCheckable(bool checkable) = 0;

	virtual bool checked() const = 0;
	virtual void setChecked(bool checked) = 0;

	virtual bool enabled() const = 0;
	virtual void setEnabled(bool enabled) = 0;
};

/******************************************************************************/

class CORE_API Panel
{
public:
	virtual ~Panel();

	virtual Button::SPtr addButton(const std::string& name, const std::string& help,
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

	virtual Button::SPtr addItem(const std::string& name, const std::string& help, CallbackFunc callback) = 0;
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

namespace buttons
{

enum {
	Ok = 0x00000400,
	Save = 0x00000800,
	SaveAll = 0x00001000,
	Open = 0x00002000,
	Yes = 0x00004000,
	YesToAll = 0x00008000,
	No = 0x00010000,
	NoToAll = 0x00020000,
	Abort = 0x00040000,
	Retry = 0x00080000,
	Ignore = 0x00100000,
	Close = 0x00200000,
	Cancel = 0x00400000,
	Discard = 0x00800000,
	Help = 0x01000000,
	Apply = 0x02000000,
	Reset = 0x04000000
};

}

/******************************************************************************/

enum class MenuType { File, Tools, View, Help };
enum class MessageBoxType { about, critical, information, question, warning };

class CORE_API SimpleGUI
{
public:
	using ObjectPropertiesPair = std::pair<GraphNode*, std::shared_ptr<ObjectProperties>>;

	virtual ~SimpleGUI();

	virtual Menu& getMenu(MenuType type) = 0;

	virtual Panel& buttonsPanel() = 0; // Access to the buttons dock (on the side of the OpenGL view)

	virtual int addStatusBarZone(const std::string& text) = 0; // Use the text parameter to set the minimum size of the zone
	virtual void setStatusBarText(int id, const std::string& text) = 0;

	virtual Dialog::SPtr createDialog(const std::string& title) = 0;
	virtual std::string getOpenFileName(const std::string& caption, const std::string& path, const std::string& filters) = 0; // Returns an empty string if the user cancels
	virtual std::string getSaveFileName(const std::string& caption, const std::string& path, const std::string& filters) = 0;
	
	// A combination of buttons can be set. Returns the button the user has clicked.
	virtual int messageBox(MessageBoxType type, const std::string& caption, const std::string& text, int buttons = buttons::Ok) = 0;

	virtual void updateView() = 0; // Update the OpenGL view

	virtual void closePropertiesDialog(GraphNode* node) = 0;
	virtual void closeAllPropertiesDialogs() = 0;
	virtual std::vector<ObjectPropertiesPair> getOpenedPropertiesDialogs() = 0;

	virtual Settings& settings() = 0;

	virtual void closeDocument() = 0; // Asks the UI to close the document (after all other events have been processed)
};

} // namespace simplegui
