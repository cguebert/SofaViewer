#pragma once

#include <serialization/serialization.h>

#include <functional>
#include <string>

class BaseDocument;

bool SERIALIZATION_API exportToXMLFile(BaseDocument& document, const std::string& filePath);
