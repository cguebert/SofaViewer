#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class BasePropertyWidget;
class MainWindow;

class DialogImpl;
class DialogImpl;
class MenuImpl;
class PanelImpl;
class SettingsImpl;
class SimpleGUIImpl;

class QLabel;

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
	using DialogImplPtr = std::shared_ptr<DialogImpl>;
	using MenuImplPtr = std::shared_ptr<MenuImpl>;
	using PanelImplPtr = std::shared_ptr<PanelImpl>;
	using SettingsImplPtr = std::shared_ptr<SettingsImpl>;

	MainWindow* m_mainWindow;
	PanelImplPtr m_buttonsPanel;
	SettingsImplPtr m_settings;
	std::vector<QLabel*> m_statusBarLabels;
	std::vector<MenuImplPtr> m_mainMenus;
	std::vector<DialogImplPtr> m_dialogs;
};
