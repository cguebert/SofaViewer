#include <serialization/DocXML.h>

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/ObjectProperties.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace
{

std::string removeWhitespaces(const std::string& val)
{
	std::string ret = val;
	std::replace(ret.begin(), ret.end(), ' ', '_');
	return ret;
}

class SERIALIZATION_API XMLExporter
{
public:
	using Attribute = std::pair<std::string, std::string>;
	using AttributesList = std::vector<Attribute>;

	XMLExporter(std::ostream& out) : m_out(out) {}

	void startNode(const std::string& type, const AttributesList& attributes);
	void endNode();
	void addObject(const std::string& type, const AttributesList& attributes);

private:
	void indent();

	std::ostream& m_out;
	std::vector<std::string> m_currentNodes;
};

void XMLExporter::startNode(const std::string& type, const XMLExporter::AttributesList& attributes)
{
	indent();
	m_out << "<" << type << " ";
	for (auto& att : attributes)
		m_out << att.first << "=\"" << att.second << "\" ";
	m_out << ">\n";

	m_currentNodes.push_back(type);
}

void XMLExporter::endNode()
{
	auto type = m_currentNodes.back();
	m_currentNodes.pop_back();
	indent();
	m_out << "</" << type << ">\n";
}

void XMLExporter::addObject(const std::string& type, const XMLExporter::AttributesList& attributes)
{
	indent();
	m_out << "<" << type << " ";
	for (auto& att : attributes)
		m_out << att.first << "=\"" << att.second << "\" ";
	m_out << "/>\n";
}

void XMLExporter::indent()
{
	for (unsigned int i = 0; i < m_currentNodes.size(); ++i)
		m_out << "\t";
}

//****************************************************************************//

XMLExporter::AttributesList getAttributes(ObjectProperties* properties)
{
	XMLExporter::AttributesList attributes;
	for (const auto& prop : properties->properties())
	{
		if (!prop->readOnly())
			attributes.emplace_back(removeWhitespaces(prop->name()), prop->value()->toString());
	}
	return attributes;
}

void addName(XMLExporter::AttributesList& attributes, const std::string& name)
{
	if (attributes.end() == std::find_if(attributes.begin(), attributes.end(),
		[&name](const XMLExporter::Attribute& attribute) { return attribute.first == name; })
		)
		attributes.emplace(attributes.begin(), "name", name);
}

void exportNode(XMLExporter& exporter, BaseDocument& document, GraphNode* node)
{
	auto properties = document.objectProperties(node);
	XMLExporter::AttributesList attributes;
	if (properties)
		attributes = getAttributes(properties.get());
	addName(attributes, node->name);

	auto type = removeWhitespaces(node->type);
	if (node->children.empty())
		exporter.addObject(type, attributes);
	else
	{
		exporter.startNode(type, attributes);
		for (const auto& child : node->children)
			exportNode(exporter, document, child.get());
		exporter.endNode();
	}
}

}

bool exportToXMLFile(BaseDocument& document, const std::string& filePath)
{
	auto root = document.graph().root();
	if (!root)
		return false;

	std::ofstream file(filePath);
	if (!file.is_open())
		return false;

	XMLExporter exporter(file);
	exportNode(exporter, document, root);
	return true;
}