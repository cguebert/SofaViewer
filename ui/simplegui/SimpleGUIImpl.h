#pragma once

#include <core/SimpleGUI.h>

#include <memory>
#include <vector>

class BaseDocument;
class BasePropertyWidget;
class MainWindow;
class ObjectProperties;
class PropertiesDialog;

class DialogImpl;
class MenuImpl;
class PanelImpl;
class SettingsImpl;
class SimpleGUIImpl;

class QLabel;
class QMainWindow;
class QMenu;
class QWidget;

class SimpleGUIImpl : public simplegui::SimpleGUI
{
public:
	SimpleGUIImpl(QMainWindow* mainWindow, QWidget* view, QWidget* buttonsPanelContainer, const std::vector<QMenu*>& menus);

	simplegui::Menu& getMenu(MenuType menuType) override;
	simplegui::Panel& buttonsPanel() override;
	int addStatusBarZone(const std::string& text) override;
	void setStatusBarText(int id, const std::string& text) override;
	simplegui::Dialog::SPtr createDialog(const std::string& title) override;
	std::string getOpenFileName(const std::string& caption, const std::string& path, const std::string& filters) override;
	std::string getSaveFileName(const std::string& caption, const std::string& path, const std::string& filters) override;
	void updateView() override;
	simplegui::Settings& settings() override;

	void clear();
	void setDocument(std::shared_ptr<BaseDocument> doc);

	void openPropertiesDialog(GraphNode* item);
	void closePropertiesDialog(ObjectProperties* objProp) override;
	void dialogFinished(PropertiesDialog* dialog, int result);

protected:
	void createButtonsPanel();

	using DialogImplPtr = std::shared_ptr<DialogImpl>;
	using MenuImplPtr = std::shared_ptr<MenuImpl>;
	using PanelImplPtr = std::shared_ptr<PanelImpl>;
	using SettingsImplPtr = std::shared_ptr<SettingsImpl>;
	using PropertiesDialogPair = std::pair<size_t, PropertiesDialog*>;

	QMainWindow* m_mainWindow;
	QWidget* m_mainView;
	QWidget* m_buttonsPanelContainer;
	std::vector<QMenu*> m_mainMenus;

	PanelImplPtr m_buttonsPanel;
	SettingsImplPtr m_settings;
	std::vector<QLabel*> m_statusBarLabels;
	std::vector<MenuImplPtr> m_menus;
	std::vector<DialogImplPtr> m_dialogs;
	std::vector<PropertiesDialogPair> m_propertiesDialogs;
	std::shared_ptr<BaseDocument> m_document;
};
