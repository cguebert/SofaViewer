#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class BasePropertyWidget;

class QGridLayout;
class QWidget;

class PanelImpl : public simplegui::Panel
{
public:
	PanelImpl(QWidget* parent, QGridLayout* layout);

	simplegui::Button::SPtr addButton(const std::string& name, const std::string& help,
									  simplegui::CallbackFunc callback,
									  int row, int column,
									  int rowSpan, int columnSpan) override;

	void addProperty(Property::SPtr property,
					 int row, int column,
					 int rowSpan, int columnSpan) override;

	void addLabel(const std::string& text,
				  int row, int column,
				  int rowSpan, int columnSpan) override;

	using PropertyWidgets = std::vector<std::shared_ptr<BasePropertyWidget>>;
	const PropertyWidgets& propertyWidgets() { return m_propertyWidgets; }

protected:
	QWidget* m_parent;
	QGridLayout* m_layout;
	PropertyWidgets m_propertyWidgets;
};
