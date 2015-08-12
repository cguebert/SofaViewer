#pragma once

#include "Property.h"

#include <functional>
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
	void modified(); /// Signal widgets using these ObjectProperties to update themselves

	using Callback = std::function<void()>;
	void addModifiedCallback(Callback func);

protected:
	std::string m_name;
	PropertyList m_properties;
	std::vector<Callback> m_modifiedCallbacks;
};

inline const std::string& ObjectProperties::name() const
{ return m_name; }

inline void ObjectProperties::addProperty(Property::PropertyPtr prop)
{ m_properties.push_back(prop); }

inline const ObjectProperties::PropertyList& ObjectProperties::properties()
{ return m_properties; }
