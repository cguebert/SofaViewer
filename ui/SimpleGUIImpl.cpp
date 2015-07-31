#include <ui/MainWindow.h>
#include <ui/SimpleGUIImpl.h>

#include <QtWidgets>

ButtonsPanel::ButtonsPanel(MainWindow* mainWindow)
	: m_mainWindow(mainWindow)
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

	auto layout = m_mainWindow->buttonsLayout();
	if(row < 0)
		row = layout ? layout->rowCount() : 0;

	layout->addWidget(button, row, column, rowSpan, columnSpan);
}

/******************************************************************************/

SimpleGUIImpl::SimpleGUIImpl(MainWindow* mainWindow)
	: m_mainWindow(mainWindow)
	, m_buttonsPanel(mainWindow)
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
	m_menuActions.push_back(action);
}

ui::Panel& SimpleGUIImpl::buttonsPanel()
{
	return m_buttonsPanel;
}

int SimpleGUIImpl::addStatusBarZone(const std::string& text)
{
	auto label = new QLabel(text.c_str());
	label->setAlignment(Qt::AlignLeft);
	label->setMinimumSize(label->sizeHint());

	m_mainWindow->statusBar()->addWidget(label);
	auto id = m_statusBarLabels.size();
	m_statusBarLabels.push_back(label);
	return id;
}

void SimpleGUIImpl::setStatusBarText(int id, const std::string& text)
{
	m_statusBarLabels[id]->setText(text.c_str());
}

void SimpleGUIImpl::clear()
{
	// Status bar
	m_mainWindow->setStatusBar(new QStatusBar);
	m_statusBarLabels.clear();

	// Menus
	for(auto action : m_menuActions)
		delete action;
	m_menuActions.clear();

	// Buttons box
	auto layout = m_mainWindow->buttonsLayout();
	auto buttonsWidget = layout->parentWidget();
	if(layout)
		delete layout;
	new QGridLayout(buttonsWidget);
}
