#include <ui/MainWindow.h>
#include <ui/SimpleGUIImpl.h>

#include <QtWidgets>

ButtonsPanel::ButtonsPanel(MainWindow* mainWindow, QGridLayout* layout)
	: m_mainWindow(mainWindow)
	, m_layout(layout)
{

}

void ButtonsPanel::addButton(const std::string& name, const std::string& help,
							 ui::CallbackFunc callback,
							 int row, int column,
							 int rowSpan, int columnSpan)
{
	auto button = new QPushButton(name.c_str());
	button->setStatusTip(help.c_str());

	int id = m_mainWindow->addCallback(callback);
	auto action = new QAction(name.c_str(), m_mainWindow);
	action->setData(QVariant(id));

	QObject::connect(button, SIGNAL(clicked(bool)), action, SLOT(trigger()));
	m_mainWindow->connect(action, SIGNAL(triggered(bool)), m_mainWindow, SLOT(executeCallback()));

	if(row < 0)
		row = m_layout->count() ? m_layout->rowCount() : 0;

	m_layout->addWidget(button, row, column, rowSpan, columnSpan);
}

/******************************************************************************/

SimpleGUIImpl::SimpleGUIImpl(MainWindow* mainWindow, QGridLayout* buttonsLayout)
	: m_mainWindow(mainWindow)
	, m_buttonsPanel(mainWindow, buttonsLayout)
{ }

void SimpleGUIImpl::addMenuItem(Menu menuId, const std::string& name, const std::string& help, ui::CallbackFunc callback)
{
	auto menu = m_mainWindow->menu(static_cast<unsigned char>(menuId));

	int id = m_mainWindow->addCallback(callback);
	auto action = new QAction(name.c_str(), menu);
	action->setStatusTip(help.c_str());
	action->setData(QVariant(id));

	m_mainWindow->connect(action, SIGNAL(triggered(bool)), m_mainWindow, SLOT(executeCallback()));
	menu->addAction(action);
}

ui::Panel& SimpleGUIImpl::buttonsPanel()
{
	return m_buttonsPanel;
}
