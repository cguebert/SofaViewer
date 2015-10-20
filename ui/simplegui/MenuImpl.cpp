#include <ui/MainWindow.h>
#include <ui/simplegui/MenuImpl.h>
#include <ui/simplegui/SimpleGUIImpl.h>

#include <QtWidgets>

#include <iostream>

MenuImpl::MenuImpl(SimpleGUIImpl* simpleGUI, QMenu* menu)
	: m_simpleGUI(simpleGUI)
	, m_menu(menu)
{
}

MenuImpl::~MenuImpl()
{
	for (auto action : m_actions)
		delete action;
}

void MenuImpl::addItem(const std::string& name, const std::string& help, simplegui::CallbackFunc callback)
{
	auto mainWindow = m_simpleGUI->mainWindow();
	int id = mainWindow->addCallback(callback);
	auto action = new QAction(name.c_str(), m_menu);
	action->setStatusTip(help.c_str());
	action->setData(QVariant(id));
	m_actions.push_back(action);

	mainWindow->connect(action, SIGNAL(triggered(bool)), mainWindow, SLOT(executeCallback()));
	m_menu->addAction(action);
}

simplegui::Menu& MenuImpl::addMenu(const std::string& name)
{
	auto menu = m_menu->addMenu(name.c_str());
	auto menuImpl = std::make_shared<MenuImpl>(m_simpleGUI, menu);
	m_subMenus.push_back(menuImpl);
	return *menuImpl;
}

void MenuImpl::addSeparator()
{
	auto action = m_menu->addSeparator();
	m_actions.push_back(action);
}
