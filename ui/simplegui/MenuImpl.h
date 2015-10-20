#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class BasePropertyWidget;
class MainWindow;
class SimpleGUIImpl;

class QAction;
class QMenu;

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
