#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class BaseDocument;
class BasePropertyWidget;
class GraphNode;
class MainWindow;
class ObjectProperties;
class PropertiesDialog;

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
	simplegui::Settings& settings() override;

	void clear();
	void setDocument(std::shared_ptr<BaseDocument> doc);

	void openPropertiesDialog(GraphNode* item);
	void closePropertiesDialog(ObjectProperties* objProp) override;
	void dialogFinished(PropertiesDialog* dialog, int result);

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
	std::shared_ptr<BaseDocument> m_document;

	using PropertiesDialogPair = std::pair<size_t, PropertiesDialog*>;
	std::vector<PropertiesDialogPair> m_propertiesDialogs;
};
