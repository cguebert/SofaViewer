#include "SofaProperties.h"
#include <core/VectorWrapper.h>

#include <iostream>

SofaObjectProperties::SofaObjectProperties(sfe::Node node)
	: ObjectProperties(node.name())
{
	auto names = node.listData();
	for(const auto& name : names)
		addData(node.data(name));
}

SofaObjectProperties::SofaObjectProperties(sfe::Object object)
	: ObjectProperties(object.name())
{
	auto names = object.listData();
	for(const auto& name : names)
		addData(object.data(name));
}

template <class T>
std::shared_ptr<BaseDataWrapper> createProp(sfe::Data data, const std::string& widget)
{
	auto prop = std::make_shared<Property>(data.name(), widget, data.readOnly(), data.help(), data.group());

	T val;
	data.get(val);
	prop->setValue(std::make_shared<PropertyValueCopy<T>>(std::move(val)));

	return std::make_shared<DataWrapper<T>>(data, prop);
}

template <class T>
std::shared_ptr<BaseDataWrapper> createVectorProp(sfe::Data data, const std::string& widget, bool fixedSize, int columnCount)
{
	auto prop = std::make_shared<Property>(data.name(), widget, data.readOnly(), data.help(), data.group());

	using value_type = std::vector<T>;
	value_type val;
	data.get(val);

	using WrapperType = VectorWrapper<value_type>;
	WrapperType wrapper(std::move(val));
	wrapper.setFixedSize(fixedSize);
	wrapper.setColumnCount(columnCount);
	prop->setValue(std::make_shared<PropertyValueCopy<WrapperType>>(std::move(wrapper)));

	return std::make_shared<VectorDataWrapper<WrapperType>>(data, prop);
}

void SofaObjectProperties::addData(sfe::Data data)
{
	if(!data || !data.displayed())
		return;

	auto storageType = data.supportedType();

	auto typeTrait = data.typeInfo();
	std::string valueType;
	int columnCount = 1;
	bool fixedSize = false;
	if(typeTrait && typeTrait->ValidInfo())
	{
		valueType = typeTrait->ValueType()->name();
		columnCount = typeTrait->size();
		fixedSize = typeTrait->FixedSize();
	}

	std::string widget;
	if(valueType == "bool")
		widget = "checkbox";

	std::shared_ptr<BaseDataWrapper> wrapper;
	switch(storageType)
	{
	case sfe::Data::DataType::Int:		wrapper = createProp<int>(data, widget);			break;
	case sfe::Data::DataType::Float:	wrapper = createProp<float>(data, widget);			break;
	case sfe::Data::DataType::Double:	wrapper = createProp<double>(data, widget);			break;
	case sfe::Data::DataType::String:	wrapper = createProp<std::string>(data, widget);	break;
	case sfe::Data::DataType::Vector_Int:		wrapper = createVectorProp<int>(data, widget, fixedSize, columnCount);			break;
	case sfe::Data::DataType::Vector_Float:		wrapper = createVectorProp<float>(data, widget, fixedSize, columnCount);		break;
	case sfe::Data::DataType::Vector_Double:	wrapper = createVectorProp<double>(data, widget, fixedSize, columnCount);		break;
	case sfe::Data::DataType::Vector_String:	wrapper = createVectorProp<std::string>(data, widget, fixedSize, columnCount);	break;
	}

	addProperty(wrapper->property());
	m_dataWrappers.push_back(wrapper);
}

void SofaObjectProperties::apply()
{
	for(auto wrapper : m_dataWrappers)
		wrapper->writeToData();
}

void SofaObjectProperties::updateProperties()
{
	for(auto wrapper : m_dataWrappers)
		wrapper->readFromData();
}
