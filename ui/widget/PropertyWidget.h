#pragma once

#include <core/Property.h>

#include <QWidget>
#include <QAbstractTableModel>

#include <memory>

// One of this will be created for each Property.
// It is an intermediary between the dialog and the widgets used to show and edit the Property
// This class is a QObject so that we can use signals and slots
class BasePropertyWidget : public QWidget
{
	Q_OBJECT
public:
	BasePropertyWidget(Property::PropertyPtr property, QWidget* parent = nullptr)
		: QWidget(parent), m_property(property) {}
	virtual ~BasePropertyWidget() {}

	std::string displayName() { return m_property->name(); }
	bool readOnly() { return m_property->readOnly(); }
	Property::PropertyPtr property() { return m_property; }

	// The implementation of this method holds the widget creation and the signal / slot connections.
	virtual QWidget* createWidgets() = 0;

public slots:
	/// Checks that widget has been edited
	void updatePropertyValue()
	{
		if(m_dirty)
			writeToProperty();
		m_dirty = false;
	}

	/// First checks that the widget is not currently being edited
	/// checks that the data has changed since the last time the widget
	/// has read the data value.
	/// ultimately read the data value.
	void updateWidgetValue()
	{
		if(!m_dirty)
		{
			readFromProperty();
			update();
		}
	}
	/// You call this slot anytime you want to specify that the widget
	/// value is out of sync with the underlying data value.
	void setWidgetDirty(bool b = true)
	{
		m_dirty = b;
		updatePropertyValue();
	}

protected:
	/// The implementation of this method tells how the widget reads the value of the property.
	virtual void readFromProperty() = 0;
	/// The implementation of this methods needs to tell how the widget can write its value in the property.
	virtual void writeToProperty() = 0;

	bool m_dirty = false;
	Property::PropertyPtr m_property;
};

/*****************************************************************************/

// Specializations for supported types
template <class T>
class PropertyWidget : public BasePropertyWidget
{
public:
	typedef T value_type;
	typedef const T& const_reference;

	PropertyWidget(Property::PropertyPtr property, QWidget* parent = nullptr)
		: BasePropertyWidget(property, parent)
		, m_propertyValue(std::dynamic_pointer_cast<PropertyValue<T>>(property->value()))
	{ }

	const_reference getValue() const
	{
		return m_propertyValue->value();
	}

	template <class U>
	void setValue(U&& value)
	{
		m_propertyValue->setValue(std::forward<U>(value));
	}

protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};
