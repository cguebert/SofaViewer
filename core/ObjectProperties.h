#pragma once

#include "Property.h"

#include <vector>

class ObjectProperties
{
public:

//protected:
	std::string m_name, m_class, m_template;
	std::vector<Property> m_properties;
};
