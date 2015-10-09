#pragma once

#include "Property.h"

#include <functional>
#include <vector>

class CORE_API ObjectProperties
{
public:
	using SPtr = std::shared_ptr<ObjectProperties>;

	ObjectProperties(const std::string& name);
	virtual ~ObjectProperties() {}

	const std::string& name() const;
	
	void addProperty(Property::SPtr prop);
	using PropertyList = std::vector<Property::SPtr>;
	const PropertyList& properties();

	void addValueWrapper(BaseValueWrapper::SPtr wrap);
	using ValueWrapperList = std::vector<BaseValueWrapper::SPtr>;
	const ValueWrapperList& valueWrappers();

	void applyProperties(); /// Save the properties
	void updateProperties(); /// Reload the properties

	void modified(); /// Signal widgets using these ObjectProperties to update themselves

	using Callback = std::function<void()>;
	void addModifiedCallback(Callback func);

protected:
	std::string m_name;
	PropertyList m_properties;
	std::vector<Callback> m_modifiedCallbacks;
	ValueWrapperList m_valueWrappers;
};

inline const std::string& ObjectProperties::name() const
{ return m_name; }

inline void ObjectProperties::addProperty(Property::SPtr prop)
{ m_properties.push_back(prop); }

inline const ObjectProperties::PropertyList& ObjectProperties::properties()
{ return m_properties; }

inline void ObjectProperties::addValueWrapper(BaseValueWrapper::SPtr wrap)
{ m_valueWrappers.push_back(wrap); }

inline const ObjectProperties::ValueWrapperList& ObjectProperties::valueWrappers()
{ return m_valueWrappers; }
