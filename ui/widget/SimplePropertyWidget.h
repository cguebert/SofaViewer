#pragma once

#include <ui/widget/PropertyWidget.h>

/// This class is used to specify how to graphically represent a data type,
template<class T>
class PropretyWidgetContainer
{
protected:
	typedef T value_type;

public:
	PropretyWidgetContainer() {}

	QWidget* createWidgets(BasePropertyWidget* parent);
	void readFromProperty(const value_type& d);
	void writeToProperty(value_type& d);
};

//****************************************************************************//

/// This class manages the GUI of a BaseData, using the corresponding instance of DataWidgetContainer
template<class T, class Container = PropretyWidgetContainer<T> >
class SimplePropertyWidget : public PropertyWidget<T>
{
protected:
	Container container;

public:
	typedef T value_type;

	SimplePropertyWidget(Property::PropertyPtr prop, QWidget* parent)
		: PropertyWidget<T>(prop, parent)
	{}

	QWidget* createWidgets() override
	{
		QWidget* w = container.createWidgets(this);
		if(!w)
			return nullptr;

		container.readFromProperty(getValue());
		return w;
	}

	void readFromProperty() override
	{
		container.readFromProperty(getValue());
	}

	void writeToProperty() override
	{
		m_resetValue = getValue();
		container.writeToProperty(m_resetValue);
		setValue(m_resetValue);
	}

	bool isModified() override
	{
		value_type tempValue = m_resetValue;
		container.writeToProperty(tempValue);
		return (tempValue != m_resetValue);
	}

	void resetWidget() override
	{
		container.readFromProperty(m_resetValue);
	}
};
