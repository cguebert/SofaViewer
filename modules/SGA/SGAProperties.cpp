#include "SGAProperties.h"

#include <core/PropertiesUtils.h>

#include <iostream>

namespace
{

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

template <class T>
BaseValueWrapper::SPtr createWrapper(sga::Property sgaProp)
{
	auto prop = createProperty(sgaProp);

	T val;
	sgaProp.get(val);
	prop->setValue(property::createCopyValue(std::move(val)));

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
	prop->setValue(property::createCopyValue(std::move(wrapper)));

	return std::make_shared<VectorPropertyWrapper<WrapperType>>(sgaProp, prop);
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
	case sgaPropType::Bool:		wrapper = createWrapper<int>(sgaProp);			break;
	case sgaPropType::Enum:		wrapper = createWrapper<int>(sgaProp);			break;
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

}

ObjectProperties::SPtr createSGAObjectProperties(sga::ObjectDefinition definition)
{
	auto properties = std::make_shared<ObjectProperties>(definition.label());
	for (auto& prop : definition.properties())
		addProperty(properties, prop);

	return properties;
}