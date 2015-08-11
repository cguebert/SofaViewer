#pragma once

#include "Property.h"

#include <vector>

class ObjectProperties
{
public:
	ObjectProperties(const std::string& name);
	virtual ~ObjectProperties() {}

	const std::string& name() const;

	using PropertyList = std::vector<Property::PropertyPtr>;

	void addProperty(Property::PropertyPtr prop);
	const PropertyList& properties();

	virtual void apply() {} /// Implementations can save the properties here

protected:
	std::string m_name;
	PropertyList m_properties;
};

inline const std::string& ObjectProperties::name() const
{ return m_name; }

inline void ObjectProperties::addProperty(Property::PropertyPtr prop)
{ m_properties.push_back(prop); }

inline const ObjectProperties::PropertyList& ObjectProperties::properties()
{ return m_properties; }
