#pragma once

#include <core/ObjectProperties.h>

#include <sga/ObjectDefinition.h>

class Property;

class BasePropertyWrapper : public BaseValueWrapper
{
public:
	BasePropertyWrapper(sga::Property sgaProp, Property::SPtr prop)
		: BaseValueWrapper(prop)
		, m_sgaProperty(sgaProp) {}

	sga::Property sgaProperty() const { return m_sgaProperty; }

protected:
	sga::Property m_sgaProperty;
};

//****************************************************************************//

template <class T>
class PropertyWrapper : public BasePropertyWrapper
{
public:
	PropertyWrapper(sga::Property sgaProp, Property::SPtr property)
		: BasePropertyWrapper(sgaProp, property)
	{ m_propertyValue = property->value<T>(); }
	void writeToValue() override
	{ T val = m_propertyValue->value(); m_sgaProperty.set(val); }
	void readFromValue() override
	{ T val; if (m_sgaProperty.get(val)) m_propertyValue->setValue(val); }
protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};

//****************************************************************************//

template <class T>
class VectorPropertyWrapper : public BasePropertyWrapper
{
public:
	using value_type = typename T::value_type;
	VectorPropertyWrapper(sga::Property sgaProp, Property::SPtr property)
		: BasePropertyWrapper(sgaProp, property)
	{ m_propertyValue = property->value<T>(); }
	void writeToValue() override
	{ value_type val = m_propertyValue->value(); m_sgaProperty.set(val); }
	void readFromValue() override
	{ value_type val; if(m_sgaProperty.get(val)) m_propertyValue->value() = val; }
protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};

//****************************************************************************//

ObjectProperties::SPtr createSGAObjectProperties(sga::ObjectDefinition definition);
