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

	void resize(int nb);
	void toggleView(bool show);

protected:
	void readFromProperty(BasePropertyValue::SPtr value);
	void writeToProperty(BasePropertyValue::SPtr value);

	QSpinBox* m_spinBox = nullptr;
	QPushButton* m_toggleButton = nullptr;
	QScrollArea* m_scrollArea = nullptr;
	QFormLayout* m_formLayout = nullptr;

	meta::Struct* m_structProperty;
	BasePropertyValue::SPtr m_value, m_resetValue;

	std::vector<const BasePropertyWidgetCreator*> m_widgetCreators;
	std::vector<std::shared_ptr<BasePropertyWidget>> m_propertyWidgets;
};
