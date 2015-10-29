#include <core/BaseDocument.h>
#include <core/DocumentFactory.h>

BaseDocument::BaseDocument(const std::string& type)
	: m_documentType(type)
{
}

std::string BaseDocument::documentType() const
{
	return m_documentType;
}

std::string BaseDocument::module() const
{
	return DocumentFactory::instance().document(m_documentType).module;
}

std::string BaseDocument::modulePath() const
{
	return DocumentFactory::instance().module(module()).dirPath;
}
