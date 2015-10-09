#pragma once

#include <core/ObjectProperties.h>

#include <sfe/Node.h>

class Property;

class BaseDataWrapper : public BaseValueWrapper
{
public:
	BaseDataWrapper(sfe::Data data, Property::SPtr property)
		: BaseValueWrapper(property)
		, m_data(data) {}

	sfe::Data data() const { return m_data; }

protected:
	sfe::Data m_data;
};

//****************************************************************************//

template <class T>
class DataWrapper : public BaseDataWrapper
{
public:
	DataWrapper(sfe::Data data, Property::SPtr property)
		: BaseDataWrapper(data, property)
	{ m_propertyValue = property->value<T>(); }
	void writeToValue() override
	{ T val = m_propertyValue->value(); m_data.set(val); }
	void readFromValue() override
	{ T val; if(m_data.get(val)) m_propertyValue->setValue(val); }
protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};

//****************************************************************************//

template <class T>
class VectorDataWrapper : public BaseDataWrapper
{
public:
	using value_type = typename T::value_type;
	VectorDataWrapper(sfe::Data data, Property::SPtr property)
		: BaseDataWrapper(data, property)
	{ m_propertyValue = property->value<T>(); }
	void writeToValue() override
	{ value_type val = m_propertyValue->value(); m_data.set(val); }
	void readFromValue() override
	{ value_type val; if(m_data.get(val)) m_propertyValue->value() = val; }
protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};

//****************************************************************************//

ObjectProperties::SPtr createSofaObjectProperties(sfe::Node node);
ObjectProperties::SPtr createSofaObjectProperties(sfe::Object object);
