#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class BasePropertyWidget;
class MainWindow;

class QGridLayout;

class PanelImpl : public simplegui::Panel
{
public:
	PanelImpl(MainWindow* mainWindow, QGridLayout* layout);

	void addButton(const std::string& name, const std::string& help,
				   simplegui::CallbackFunc callback,
				   int row, int column,
				   int rowSpan, int columnSpan) override;

	void addProperty(Property::SPtr property,
								 int row, int column,
								 int rowSpan, int columnSpan) override;

	using PropertyWidgets = std::vector<std::shared_ptr<BasePropertyWidget>>;
	const PropertyWidgets& propertyWidgets() { return m_propertyWidgets; }

protected:
	MainWindow* m_mainWindow;
	QGridLayout* m_layout;
	PropertyWidgets m_propertyWidgets;
};
