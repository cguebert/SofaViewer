#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class BasePropertyWidget;
class MainWindow;
class SimpleGUIImpl;

class QDialog;
class QGridLayout;
class QLabel;
class QLayout;
class QMenu;
class QSettings;

class PanelImpl : public simplegui::Panel
{
public:
	PanelImpl(MainWindow* mainWindow, QGridLayout* layout);

	void addButton(const std::string& name, const std::string& help,
				   simplegui::CallbackFunc callback,
				   int row, int column,
				   int rowSpan, int columnSpan) override;

	void addProperty(Property::SPtr property,
								 int row, int column,
								 int rowSpan, int columnSpan) override;

	using PropertyWidgets = std::vector<std::shared_ptr<BasePropertyWidget>>;
	const PropertyWidgets& propertyWidgets() { return m_propertyWidgets; }

protected:
	MainWindow* m_mainWindow;
	QGridLayout* m_layout;
	PropertyWidgets m_propertyWidgets;
};

/******************************************************************************/

class DialogImpl : public simplegui::Dialog
{
public:
	DialogImpl(MainWindow* mainWindow, const std::string& title);

	simplegui::Panel& content() override;
	bool exec() override;
	void show() override;

	void close();

protected:
	void completeLayout();

	QDialog* m_dialog;
	QGridLayout* m_panelLayout;
	std::shared_ptr<PanelImpl> m_dialogPanel;
};

/******************************************************************************/

class MenuImpl : public simplegui::Menu
{
public:
	using SPtr = std::shared_ptr<MenuImpl>;
	MenuImpl(SimpleGUIImpl* simpleGUI, QMenu* menu);
	~MenuImpl();

	void addItem(const std::string& name, const std::string& help, simplegui::CallbackFunc callback) override;
	simplegui::Menu& addMenu(const std::string& name) override;
	void addSeparator() override;

protected:
	SimpleGUIImpl* m_simpleGUI;
	QMenu* m_menu;
	std::vector<SPtr> m_subMenus;
	std::vector<QAction*> m_actions;
};

/******************************************************************************/

class SettingsImpl : public simplegui::Settings
{
public:
	SettingsImpl(MainWindow* mainWindow);

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

/******************************************************************************/

class SimpleGUIImpl : public simplegui::SimpleGUI
{
public:
	SimpleGUIImpl(MainWindow* mainWindow);

	simplegui::Menu& getMenu(MenuType menuType) override;
	simplegui::Panel& buttonsPanel() override;
	int addStatusBarZone(const std::string& text) override;
	void setStatusBarText(int id, const std::string& text) override;
	simplegui::Dialog::SPtr createDialog(const std::string& title) override;
	void updateView() override;
	void closePropertiesDialog(ObjectProperties* objProp) override;
	simplegui::Settings& settings() override;

	void clear();
	void setDocumentType(const std::string& type);

	MainWindow* mainWindow();

protected:
	MainWindow* m_mainWindow;
	PanelImpl m_buttonsPanel;
	std::vector<QLabel*> m_statusBarLabels;
	std::vector<MenuImpl::SPtr> m_mainMenus;
	using DialogImplPtr = std::shared_ptr<DialogImpl>;
	std::vector<DialogImplPtr> m_dialogs;
	SettingsImpl m_settings;
};
