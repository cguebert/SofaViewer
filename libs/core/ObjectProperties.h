#pragma once

#include "Property.h"
#include "PropertiesUtils.h"

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
	const PropertyList& properties() const;
	Property::SPtr property(const std::string& name) const;

	void addValueWrapper(BaseValueWrapper::SPtr wrap);
	using ValueWrapperList = std::vector<BaseValueWrapper::SPtr>;
	const ValueWrapperList& valueWrappers();

	/// Helper method to create a property that will directly modify the value passed (don't pass temporaries!)
	/// Will create a ValueWrapper if the value passed is too complex to be used directly
	template <class T, class... MetaArgs>
	Property::SPtr createRefProperty(const std::string& name, T& val, MetaArgs&&... meta) 
	{
		auto propWrapperPair = property::createRefPropertyWrapperPair(name, val, std::forward<MetaArgs>(meta)...);
		addProperty(propWrapperPair.first);
		if(propWrapperPair.second)
			addValueWrapper(propWrapperPair.second);
		return propWrapperPair.first;
	}

	template <class T, class... MetaArgs>
	Property::SPtr createCopyProperty(const std::string& name, T&& val, MetaArgs&&... meta) /// Helper method to create a property that will copy the value passed (can give temporaries)
	{
		auto prop = property::createCopyProperty(name, std::forward<T>(val), std::forward<MetaArgs>(meta)...);
		addProperty(prop);
		return prop;
	}

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

inline const ObjectProperties::PropertyList& ObjectProperties::properties() const
{ return m_properties; }

inline void ObjectProperties::addValueWrapper(BaseValueWrapper::SPtr wrap)
{ m_valueWrappers.push_back(wrap); }

inline const ObjectProperties::ValueWrapperList& ObjectProperties::valueWrappers()
{ return m_valueWrappers; }
