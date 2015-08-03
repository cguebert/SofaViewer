#include <core/Property.h>

Property::Property()
	: m_type(std::type_index(typeid(int)))
{
}

Property::Property(const std::string& name, ValuePtr value)
	: m_name(name)
	, m_value(value)
	, m_type(value->type())
{
}

Property::Property(const std::string& name,
				   const std::string& widget,
				   bool readOnly,
				   const std::string& help,
				   const std::string& group)
	: m_name(name)
	, m_help(help)
	, m_group(group)
	, m_widget(widget)
	, m_readOnly(readOnly)
	, m_type(std::type_index(typeid(std::string)))
{
}

void Property::setValue(ValuePtr value)
{
	m_value = value;
	if(value)
		m_type = value->type();
}
