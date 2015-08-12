#pragma once

#include <core/ObjectProperties.h>
#include <core/VectorWrapper.h>

#include <sfe/Node.h>

class Property;

class BaseDataWrapper
{
public:
	using PropertyPtr = std::shared_ptr<Property>;

	BaseDataWrapper(sfe::Data data, PropertyPtr property)
		: m_data(data), m_property(property) {}
	~BaseDataWrapper() {}

	virtual void writeToData() = 0;
	virtual void readFromData() = 0;

	sfe::Data data() const { return m_data; }
	PropertyPtr property() const { return m_property; }

protected:
	sfe::Data m_data;
	PropertyPtr m_property;
};

//****************************************************************************//

template <class T>
class DataWrapper : public BaseDataWrapper
{
public:
	DataWrapper(sfe::Data data, PropertyPtr property)
		: BaseDataWrapper(data, property)
	{ m_propertyValue = property->value<T>(); }
	void writeToData() override
	{ T val = m_propertyValue->value(); m_data.set(val); }
	void readFromData() override
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
	VectorDataWrapper(sfe::Data data, PropertyPtr property)
		: BaseDataWrapper(data, property)
	{ m_propertyValue = property->value<T>(); }
	void writeToData() override
	{ value_type val = m_propertyValue->value(); m_data.set(val); }
	void readFromData() override
	{ value_type val; if(m_data.get(val)) m_propertyValue->value() = val; }
protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};

//****************************************************************************//

class SofaObjectProperties : public ObjectProperties
{
public:
	SofaObjectProperties(sfe::Node node);
	SofaObjectProperties(sfe::Object object);

	void apply() override; // Property -> Data
	void updateProperties(); // Data -> Property

protected:
	void addData(sfe::Data data);

	std::vector<std::shared_ptr<BaseDataWrapper>> m_dataWrappers;
};
