#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class QAction;
class QMenu;

class MenuImpl : public simplegui::Menu
{
public:
	using SPtr = std::shared_ptr<MenuImpl>;
	MenuImpl(QMenu* menu, bool freeActions = false);
	~MenuImpl();

	void addItem(const std::string& name, const std::string& help, simplegui::CallbackFunc callback) override;
	simplegui::Menu& addMenu(const std::string& name) override;
	void addSeparator() override;

protected:
	bool m_freeActions; // Should we delete the actions in the destructor ? (necessary when adding them to a menu that has other actions that must stay)
	QMenu* m_menu;
	std::vector<SPtr> m_subMenus;
	std::vector<QAction*> m_actions;
};
