#include <ui/simplegui/SettingsImpl.h>

#include <QSettings>

SettingsImpl::SettingsImpl(QObject* parent)
	: m_settings(new QSettings(parent))
{
}

void SettingsImpl::setDocumentType(const std::string& type)
{
	if(!m_documentType.empty())
		m_settings->endGroup();

	m_documentType = type;

	if(!type.empty())
		m_settings->beginGroup(type.c_str());
}

void SettingsImpl::set(const std::string& name, int val)
{
	m_settings->setValue(name.c_str(), val);
}

void SettingsImpl::set(const std::string& name, double val)
{
	m_settings->setValue(name.c_str(), val);
}

void SettingsImpl::set(const std::string& name, const std::string& val)
{
	m_settings->setValue(name.c_str(), val.c_str());
}

void SettingsImpl::set(const std::string& name, const std::vector<int>& val)
{
	QList<QVariant> list;
	for(const auto& v : val)
		list.push_back(v);
	m_settings->setValue(name.c_str(), list);
}

void SettingsImpl::set(const std::string& name, const std::vector<double>& val)
{
	QList<QVariant> list;
	for(const auto& v : val)
		list.push_back(v);
	m_settings->setValue(name.c_str(), list);
}

void SettingsImpl::set(const std::string& name, const std::vector<std::string>& val)
{
	QList<QVariant> list;
	for(const auto& v : val)
		list.push_back(v.c_str());
	m_settings->setValue(name.c_str(), list);
}

bool SettingsImpl::get(const std::string& name, int& val)
{
	auto var = m_settings->value(name.c_str());
	if(!var.isValid())
		return false;

	bool ok = false;
	val = var.toInt(&ok);
	return ok;
}

bool SettingsImpl::get(const std::string& name, double& val)
{
	auto var = m_settings->value(name.c_str());
	if(!var.isValid())
		return false;

	bool ok = false;
	val = var.toDouble(&ok);
	return ok;
}

bool SettingsImpl::get(const std::string& name, std::string& val)
{
	auto var = m_settings->value(name.c_str());
	if(!var.isValid())
		return false;

	val = var.toString().toStdString();
	return true;
}

bool SettingsImpl::get(const std::string& name, std::vector<int>& val)
{
	auto var = m_settings->value(name.c_str());
	if(!var.isValid())
		return false;

	auto list = var.toList();

	val.clear();
	for(const auto& v : list)
	{
		bool ok = false;
		val.push_back(v.toInt(&ok));
		if(!ok)
			return false;
	}
	return true;
}

bool SettingsImpl::get(const std::string& name, std::vector<double>& val)
{
	auto var = m_settings->value(name.c_str());
	if(!var.isValid())
		return false;

	auto list = var.toList();

	val.clear();
	for(const auto& v : list)
	{
		bool ok = false;
		val.push_back(v.toDouble(&ok));
		if(!ok)
			return false;
	}
	return true;
}

bool SettingsImpl::get(const std::string& name, std::vector<std::string>& val)
{
	auto var = m_settings->value(name.c_str());
	if(!var.isValid())
		return false;

	auto list = var.toList();

	val.clear();
	for(const auto& v : list)
		val.push_back(v.toString().toStdString());
	return true;
}
