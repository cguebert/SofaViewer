#pragma once

#include <core/Property.h>

#include <functional>
#include <string>
#include <vector>

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
};

/******************************************************************************/

class SimpleGUI
{
public:
	virtual ~SimpleGUI() {}

	enum class Menu : unsigned char
	{ File, Tools, View, Help };

	virtual void addMenuItem(Menu menu, const std::string& name, const std::string& help, CallbackFunc callback) = 0;
	virtual Panel& buttonsPanel() = 0;
};

} // namespace ui
