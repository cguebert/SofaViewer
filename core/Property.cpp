#include <core/Property.h>

Property::Property()
	: m_type(std::type_index(typeid(int)))
{
}

Property::Property(const std::string& name,
				   const std::string& help,
				   const std::string& group,
				   bool readOnly,
				   std::type_index type,
				   const std::string& valueType)
	: m_name(name)
	, m_help(help)
	, m_group(group)
	, m_readOnly(readOnly)
	, m_type(type)
	, m_valueType(valueType)
{
}

void Property::setValue(ValuePtr value)
{
	m_value = value;
}
