#pragma once

#include <ui/widget/PropertyWidget.h>
#include <core/StructMeta.h>

class BasePropertyWidgetCreator;

class QFormLayout;
class QPushButton;
class QScrollArea;
class QSpinBox;

class StructPropertyWidget : public BasePropertyWidget
{
public:
	StructPropertyWidget(Property::SPtr prop, QWidget* parent);

	QWidget* createWidgets() override;
	void readFromProperty() override;
	void writeToProperty() override;
	bool isModified() override;
	void resetWidget() override;
	void validate() override;

	void resize();
	void toggleView(bool show);

protected:
	QSpinBox* m_spinBox = nullptr;
	QPushButton* m_toggleButton = nullptr;
	QScrollArea* m_scrollArea = nullptr;
	QFormLayout* m_formLayout = nullptr;

	meta::Struct* m_structProperty;

	std::vector<const BasePropertyWidgetCreator*> m_widgetCreators;
	std::vector<std::shared_ptr<BasePropertyWidget>> m_propertyWidgets;
};
