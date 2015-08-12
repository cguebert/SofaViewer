#include <core/ObjectProperties.h>

#include <algorithm>

ObjectProperties::ObjectProperties(const std::string& name)
	: m_name(name)
{

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
