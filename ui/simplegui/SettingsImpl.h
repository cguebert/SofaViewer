#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class QObject;
class QSettings;

class SettingsImpl : public simplegui::Settings
{
public:
	SettingsImpl(QObject* parent = nullptr);

	void setDocumentType(const std::string& type);

	void set(const std::string& name, int val) override;
	void set(const std::string& name, double val) override;
	void set(const std::string& name, const std::string& val) override;
	void set(const std::string& name, const std::vector<int>& val) override;
	void set(const std::string& name, const std::vector<double>& val) override;
	void set(const std::string& name, const std::vector<std::string>& val) override;

	bool get(const std::string& name, int& val) override;
	bool get(const std::string& name, double& val) override;
	bool get(const std::string& name, std::string& val) override;
	bool get(const std::string& name, std::vector<int>& val) override;
	bool get(const std::string& name, std::vector<double>& val) override;
	bool get(const std::string& name, std::vector<std::string>& val) override;

protected:
	QSettings* m_settings;
	std::string m_documentType;
};
