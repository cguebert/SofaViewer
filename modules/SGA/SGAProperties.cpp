#include "SGAProperties.h"

#include <core/PropertiesUtils.h>

#include <iostream>
#include <sstream>

namespace
{

std::vector<std::string> split(const std::string& text, char delim) 
{
	std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, delim))
        tokens.push_back(item);
    return tokens;
}

bool isHidden(const std::string& name)
{
	static std::vector<std::string> reserved = { "dataPath", "displayFlags", "edges", "exportedMesh",
			"extendMax", "extendMin", /*"groups",*/ "name", "rotation", "scale",
			"self", "topologyType", "translation", "triangles", "vertices" };

	return reserved.end() != std::find(reserved.begin(), reserved.end(), name);
}

enum class sgaPropType
{
	Int, Float, String, Bool, Enum,
	Vector_Int, Vector_Float, Vector_Bool,
	Vec3d, Vec3i, Rigid,
	Vertices, Edges, Triangles, Quads, Tetrahedra, Hexahedra,
	VerticesId,
	None
};

sgaPropType getPropType(const std::string& type)
{
	static std::vector<std::string> typeNames = { "int", "float", "string", "bool", "enum", 
		"vector_int", "vector_float", "vector_bool",
		"Vec3d", "Vec3i", "Rigid", 
		"Vertices", "Edges", "Triangles", "Quads", "Tetrahedra", "Hexahedra", 
		"VerticesId" };

	auto it = std::find(typeNames.begin(), typeNames.end(), type);
	if (it == typeNames.end())
		return sgaPropType::None;
	int index = std::distance(typeNames.begin(), it);
	return static_cast<sgaPropType>(index);
}

std::string getAttribute(sga::Property sgaProp, const std::string& name)
{
	const sga::Property::AttributesList& list = sgaProp.attributes();
	auto it = list.find(name);
	if (it != list.end())
		return it->second;

	return "";
}

Property::SPtr createProperty(sga::Property sgaProp)
{
	std::string group = sgaProp.isAdvanced() ? "advanced" : "";
	std::string name = getAttribute(sgaProp, "label");
	if (name.empty())
		name = sgaProp.id();
	std::string help = getAttribute(sgaProp, "description");
	return std::make_shared<Property>(name, false, help, group);
}

template <class T> T fromString(const std::string& text);
template <> int fromString(const std::string& text) { return std::stoi(text); }
template <> unsigned int fromString(const std::string& text) { return std::stoi(text); }
template <> float fromString(const std::string& text) { return std::stof(text); }
template <> double fromString(const std::string& text) { return std::stod(text); }

template<class T, class value_type = T>
void addMeta(sga::Property sgaProp, BasePropertyValue::SPtr valuePtr)
{
	const auto& attributes = sgaProp.attributes();
	auto& metaContainer = dynamic_cast<meta::MetaContainer<T>&>(valuePtr->baseMetaContainer());

	auto minIt = attributes.find("min"), maxIt = attributes.find("max");
	if (minIt != attributes.end() || maxIt != attributes.end())
	{
		value_type minVal = minIt != attributes.end() ? fromString<value_type>(minIt->second) : std::numeric_limits<value_type>::lowest();
		value_type maxVal = maxIt != attributes.end() ? fromString<value_type>(maxIt->second) : std::numeric_limits<value_type>::max();
		metaContainer.add(meta::Range<value_type>(minVal, maxVal));
	}
}

template<>
void addMeta<std::string, std::string>(sga::Property sgaProp, BasePropertyValue::SPtr valuePtr)
{}

template <class T, class... MetaArgs>
BaseValueWrapper::SPtr createWrapper(sga::Property sgaProp, MetaArgs&&... meta)
{
	auto prop = createProperty(sgaProp);

	T val;
	sgaProp.get(val);
	auto value = property::createCopyValue(std::move(val), std::forward<MetaArgs>(meta)...);
	addMeta<T>(sgaProp, value);
	prop->setValue(value);

	return std::make_shared<PropertyWrapper<T>>(sgaProp, prop);
}

template <class T>
BaseValueWrapper::SPtr createVectorWrapper(sga::Property sgaProp, bool fixedSize, int columnCount)
{
	auto prop = createProperty(sgaProp);

	using value_type = std::vector<T>;
	value_type val;
	sgaProp.get(val);

	using WrapperType = VectorWrapper<value_type>;
	WrapperType wrapper(std::move(val));
	wrapper.setFixedSize(fixedSize);
	wrapper.setColumnCount(columnCount);
	auto value = property::createCopyValue(std::move(wrapper));
	addMeta<WrapperType, T>(sgaProp, value);
	prop->setValue(value);

	return std::make_shared<VectorPropertyWrapper<WrapperType>>(sgaProp, prop);
}

BaseValueWrapper::SPtr createEnumProperty(sga::Property sgaProp)
{
	const auto& attributes = sgaProp.attributes();
	auto valuesIt = attributes.find("values");
	if (valuesIt != attributes.end())
	{
		auto valuesText = valuesIt->second;
		if(valuesText.empty())
			return createWrapper<int>(sgaProp);

		auto values = split(valuesText, ';');
		return createWrapper<int>(sgaProp, meta::Enum(values));
	}
	else
		return createWrapper<int>(sgaProp);
}

void addProperty(ObjectProperties::SPtr properties, sga::Property sgaProp)
{
	if (isHidden(sgaProp.id()))
		return;

	BaseValueWrapper::SPtr wrapper;

	switch (getPropType(sgaProp.type()))
	{
	case sgaPropType::Int:		wrapper = createWrapper<int>(sgaProp);			break;
	case sgaPropType::Float:	wrapper = createWrapper<float>(sgaProp);		break;
	case sgaPropType::String:	wrapper = createWrapper<std::string>(sgaProp);	break;
	case sgaPropType::Bool:		wrapper = createWrapper<int>(sgaProp, meta::Checkbox());		break;
	case sgaPropType::Enum:		wrapper = createEnumProperty(sgaProp);			break;
	case sgaPropType::Vector_Int:	wrapper = createVectorWrapper<int>(sgaProp, false, 1);		break;
	case sgaPropType::Vector_Float:	wrapper = createVectorWrapper<float>(sgaProp, false, 1);	break;
	case sgaPropType::Vector_Bool:	wrapper = createVectorWrapper<int>(sgaProp, false, 1);		break;
	case sgaPropType::Vec3d:		wrapper = createVectorWrapper<double>(sgaProp, true, 3);	break;
	case sgaPropType::Vec3i:		wrapper = createVectorWrapper<int>(sgaProp, true, 3);		break;
	case sgaPropType::Rigid:		wrapper = createVectorWrapper<double>(sgaProp, true, 7);	break;
	case sgaPropType::Vertices:		wrapper = createVectorWrapper<double>(sgaProp, false, 3);	break;
	case sgaPropType::Edges:		wrapper = createVectorWrapper<unsigned int>(sgaProp, false, 2);		break;
	case sgaPropType::Triangles:	wrapper = createVectorWrapper<unsigned int>(sgaProp, false, 3);		break;
	case sgaPropType::Quads:		wrapper = createVectorWrapper<unsigned int>(sgaProp, false, 4);		break;
	case sgaPropType::Tetrahedra:	wrapper = createVectorWrapper<unsigned int>(sgaProp, false, 4);		break;
	case sgaPropType::Hexahedra:	wrapper = createVectorWrapper<unsigned int>(sgaProp, false, 8);		break;
	case sgaPropType::VerticesId:	wrapper = createVectorWrapper<unsigned int>(sgaProp, false, 1);		break;
	case sgaPropType::None:	std::cerr << "Unknown SGA property type: " << sgaProp.type() << std::endl;
	}

	if (wrapper)
	{
		properties->addProperty(wrapper->property());
		properties->addValueWrapper(wrapper);
	}
}

} // unnamed namespace

ObjectProperties::SPtr createSGAObjectProperties(sga::ObjectDefinition definition)
{
	auto properties = std::make_shared<ObjectProperties>(definition.label());
	for (auto& prop : definition.properties())
		addProperty(properties, prop);

	return properties;
}