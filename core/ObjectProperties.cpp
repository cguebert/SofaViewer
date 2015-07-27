#include <core/ObjectProperties.h>

ObjectProperties::ObjectProperties(const std::string& objectName,
								   const std::string& className,
								   const std::string& templateName)
	: m_name(objectName)
	, m_class(className)
	, m_template(templateName)
{

}
