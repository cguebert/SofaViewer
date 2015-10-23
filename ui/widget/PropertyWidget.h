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
	BasePropertyWidget(Property::SPtr property, QWidget* parent = nullptr)
		: QWidget(parent)
		, m_property(property)
		, m_state(State::unchanged)
		, m_saveTrigger(SaveTrigger::action)
	{}
	virtual ~BasePropertyWidget() {}

	std::string displayName() const { return m_property->name(); }
	bool readOnly() const { return m_property->readOnly(); }
	Property::SPtr property() { return m_property; }

	enum class State { unchanged, modified, conflict };
	State state() const { return m_state; }

	enum class Source { property, widget };
	void resolveConflict(Source source);

	enum class SaveTrigger { action, asap }; // action = when updatePropertyValue is called, asap = when the widget is modified
	void setSaveTrigger(SaveTrigger trigger);

	/// The implementation of this method holds the widget creation and the signal / slot connections.
	virtual QWidget* createWidgets() = 0;

	/// Reset the value of the widget to the one in the property.
	virtual void resetWidget() = 0;

public slots:
	/// Checks that widget has been edited
	void updatePropertyValue();

	/// First checks that the widget is not currently being edited
	/// checks that the property has changed since the last time the widget
	/// has read the property value. Ultimately read the property value.
	void updateWidgetValue();

	/// You call this slot anytime you want to specify that the widget
	/// value is out of sync with the underlying property value.
	void setWidgetDirty();

signals:
	void stateChanged(BasePropertyWidget*, int); /// Sent when the state of the widget has changed (ie. modified)

protected:
	/// The widget must read the value of the property.
	virtual void readFromProperty() = 0;
	/// The widget must write its value in the property.
	virtual void writeToProperty() = 0;
	/// Test if the value of the widget is different than the one in the property.
	virtual bool isModified() = 0;
	/// Can apply validators here
	virtual void validate() { }

	void setState(State state);

	Property::SPtr m_property;
	State m_state;
	SaveTrigger m_saveTrigger;
};

/*****************************************************************************/

// Specializations for supported types
template <class T>
class PropertyWidget : public BasePropertyWidget
{
public:
	typedef T value_type;
	typedef const T& const_reference;

	PropertyWidget(Property::SPtr property, QWidget* parent = nullptr)
		: BasePropertyWidget(property, parent)
		, m_propertyValue(std::dynamic_pointer_cast<PropertyValue<T>>(property->value()))
		, m_resetValue(m_propertyValue->value())
	{ }

	const_reference getValue() const
	{ return m_propertyValue->value(); }

	template <class U>
	void setValue(U&& value)
	{ m_propertyValue->setValue(std::forward<U>(value)); }

	const_reference resetValue() const
	{ return m_resetValue; }

	PropertyValue<T>* propertyValue() const
	{ return m_propertyValue.get(); }

protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
	value_type m_resetValue; // A copy of the value, to test for modifications
};
