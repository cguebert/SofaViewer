#pragma once

#include <serialization/serialization.h>

#include <core/BaseDocument.h>
#include <core/Graph.h>

#include <functional>
#include <string>


bool SERIALIZATION_API exportToXMLFile(BaseDocument& document, const std::string& filePath);

using NodeCreationFunc = std::function<GraphNode::SPtr(const std::string& type, const std::string& name)>;
GraphNode::SPtr SERIALIZATION_API importXMLFile(BaseDocument& document, const std::string& filePath, NodeCreationFunc nodeCreationFunc);
