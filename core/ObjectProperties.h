#pragma once

#include "Property.h"

#include <vector>

class ObjectProperties
{
public:
	ObjectProperties(const std::string& objectName,
					 const std::string& className = "",
					 const std::string& templateName = "");
	const std::string& objectName() const;
	const std::string& className() const;
	const std::string& templateName() const;

	using PropertyList = std::vector<Property::PropertyPtr>;

	void addProperty(Property::PropertyPtr prop);
	const PropertyList& properties();

protected:
	std::string m_name, m_class, m_template;
	PropertyList m_properties;
};

inline const std::string& ObjectProperties::objectName() const
{ return m_name; }

inline const std::string& ObjectProperties::className() const
{ return m_class; }

inline const std::string& ObjectProperties::templateName() const
{ return m_template; }

inline void ObjectProperties::addProperty(Property::PropertyPtr prop)
{ m_properties.push_back(prop); }

inline const ObjectProperties::PropertyList& ObjectProperties::properties()
{ return m_properties; }
