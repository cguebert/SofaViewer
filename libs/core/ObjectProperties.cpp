#include <core/ObjectProperties.h>

#include <algorithm>

ObjectProperties::ObjectProperties(const std::string& name)
	: m_name(name)
{
}

Property::SPtr ObjectProperties::property(const std::string& name) const
{
	auto it = std::find_if(m_properties.begin(), m_properties.end(), [name](const Property::SPtr& ptr) {
		return ptr->name() == name;
	});

	if (it != m_properties.end())
		return *it;

	return nullptr;
}

void ObjectProperties::applyProperties()
{
	for (auto wrapper : m_valueWrappers)
		wrapper->writeToValue();
}

void ObjectProperties::updateProperties()
{
	for (auto wrapper : m_valueWrappers)
		wrapper->readFromValue();
}

void ObjectProperties::modified()
{
	for(const auto& func : m_modifiedCallbacks)
		func();
}

void ObjectProperties::addModifiedCallback(Callback func)
{
	m_modifiedCallbacks.push_back(func);
}
