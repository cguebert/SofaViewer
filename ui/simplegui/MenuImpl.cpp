#include <ui/simplegui/MenuImpl.h>

#include <QtWidgets>

MenuImpl::MenuImpl(QMenu* menu, bool freeActions)
	: m_menu(menu)
	, m_freeActions(freeActions)
{
}

MenuImpl::~MenuImpl()
{
	for (auto action : m_actions)
		delete action;
}

void MenuImpl::addItem(const std::string& name, const std::string& help, simplegui::CallbackFunc callback)
{
	auto action = new QAction(name.c_str(), m_menu);
	action->setStatusTip(help.c_str());
	if (m_freeActions)
		m_actions.push_back(action);

	QObject::connect(action, &QAction::triggered, callback);
	m_menu->addAction(action);
}

simplegui::Menu& MenuImpl::addMenu(const std::string& name)
{
	auto menu = m_menu->addMenu(name.c_str());
	auto menuImpl = std::make_shared<MenuImpl>(menu);
	m_subMenus.push_back(menuImpl);
	return *menuImpl;
}

void MenuImpl::addSeparator()
{
	auto action = m_menu->addSeparator();
	if (m_freeActions)
		m_actions.push_back(action);
}
