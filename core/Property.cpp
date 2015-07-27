#include <core/Property.h>

Property::Property(const std::string& name,
				   const std::string& help,
				   const std::string& group,
				   bool readOnly,Type storageType,
				   const std::string& valueType)
	: m_name(name)
	, m_help(help)
	, m_group(group)
	, m_readOnly(readOnly)
	, m_storageType(storageType)
	, m_valueType(valueType)
{
}

void Property::setValue(ValuePtr value)
{
	m_value = value;
}
