#include <serialization/DocXML.h>

#include <core/ObjectProperties.h>

#include <algorithm>
#include <fstream>
#include <vector>

#include <tinyxml2.h>

using namespace tinyxml2;

namespace
{

std::string removeWhitespaces(const std::string& val)
{
	std::string ret = val;
	std::replace(ret.begin(), ret.end(), ' ', '_');
	return ret;
}

std::string removeUnderscores(const std::string& val)
{
	std::string ret = val;
	std::replace(ret.begin(), ret.end(), '_', ' ');
	return ret;
}

class XMLImporter
{
public:
	XMLImporter(BaseDocument& document, NodeCreationFunc nodeCreationFunc) 
		: m_document(document), m_nodeCreationFunc(nodeCreationFunc) {}

	GraphNode::SPtr parseNode(XMLElement* xmlElt, GraphNode* parent = nullptr);

private:
	GraphNode::SPtr createNode(XMLElement* xmlElt, GraphNode* parent);
	void fillProperties(XMLElement* xmlElt, ObjectProperties* properties);

	BaseDocument& m_document;
	NodeCreationFunc m_nodeCreationFunc;
};

GraphNode::SPtr XMLImporter::parseNode(XMLElement* xmlElt, GraphNode* parent)
{
	auto node = createNode(xmlElt, parent);

	auto xmlChild = xmlElt->FirstChildElement();
	while (xmlChild)
	{
		parseNode(xmlChild, node.get());
		xmlChild = xmlChild->NextSiblingElement();
	}

	return node;
}

GraphNode::SPtr XMLImporter::createNode(XMLElement* xmlElt, GraphNode* parent)
{
	auto type = removeUnderscores(xmlElt->Name());
	auto nameAtt = xmlElt->Attribute("name");
	auto name = nameAtt ? nameAtt : "";
	auto node = m_nodeCreationFunc(type, name);

	if (node)
	{
		node->type = type;
		node->parent = parent;
		if (parent)
			parent->children.push_back(node);

		auto properties = m_document.objectProperties(node.get());
		if (properties)
			fillProperties(xmlElt, properties.get());
	}

	return node;
}

void XMLImporter::fillProperties(XMLElement* xmlElt, ObjectProperties* properties)
{
	for (auto& prop : properties->properties())
	{
		auto attName = removeWhitespaces(prop->name());
		auto att = xmlElt->Attribute(attName.c_str());
		if (att)
			prop->value()->fromString(att);
	}

	properties->applyProperties();
}

}

GraphNode::SPtr importXMLFile(BaseDocument& document, const std::string& filePath, NodeCreationFunc nodeCreationFunc)
{
	if (!nodeCreationFunc)
		return nullptr;

	XMLDocument xmlDoc;
	if (XML_SUCCESS != xmlDoc.LoadFile(filePath.c_str()))
		return nullptr;

	XMLImporter importer(document, nodeCreationFunc);
	return importer.parseNode(xmlDoc.RootElement());
}