#pragma once

#include <serialization/serialization.h>

#include <core/Graph.h>
#include <core/ObjectProperties.h>

#include <functional>
#include <string>

using GetPropertiesFunc = std::function<ObjectProperties::SPtr(GraphNode* item)>;
bool SERIALIZATION_API exportToXMLFile(const std::string& filePath, GraphNode* item, GetPropertiesFunc getPropertiesFunc);

using NodeCreationFunc = std::function<GraphNode::SPtr(const std::string& type, const std::string& name)>;
GraphNode::SPtr SERIALIZATION_API importXMLFile(const std::string& filePath, NodeCreationFunc nodeCreationFunc, GetPropertiesFunc getPropertiesFunc);
