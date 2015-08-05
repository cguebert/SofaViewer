#include <ui/MainWindow.h>
#include <ui/SimpleGUIImpl.h>
#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <QtWidgets>

PanelImpl::PanelImpl(MainWindow* mainWindow, QGridLayout* layout)
	: m_mainWindow(mainWindow)
	, m_layout(layout)
{

}

void PanelImpl::addButton(const std::string& name, const std::string& help,
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

void PanelImpl::addProperty(Property::PropertyPtr property,
							int row, int column,
							int rowSpan, int columnSpan)
{
	std::shared_ptr<BasePropertyWidget> propWidget = PropertyWidgetFactory::instance().create(property, m_mainWindow);
	m_propertyWidgets.push_back(propWidget);

	auto widget = propWidget->createWidgets();

	if(row < 0)
		row = m_layout->count() ? m_layout->rowCount() : 0;

	if(!property->name().empty())
	{
		auto containerLayout = new QHBoxLayout;
		auto label = new QLabel(property->name().c_str());
		containerLayout->addWidget(label);
		containerLayout->addWidget(widget);
		m_layout->addLayout(containerLayout, row, column, rowSpan, columnSpan);
	}
	else
		m_layout->addWidget(widget, row, column, rowSpan, columnSpan);
}

/******************************************************************************/

DialogImpl::DialogImpl(MainWindow* mainWindow, const std::string& title)
{
	m_dialog = new QDialog(mainWindow);
	m_dialog->setWindowFlags(m_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	m_dialog->setWindowTitle(title.c_str());

	m_panelLayout = new QGridLayout;
	m_dialogPanel = std::make_shared<PanelImpl>(mainWindow, m_panelLayout);
}

ui::Panel& DialogImpl::content()
{
	return *m_dialogPanel.get();
}

bool DialogImpl::exec()
{
	completeLayout();
	auto result = m_dialog->exec();
	if(result)
	{
		for(const auto& widget : m_dialogPanel->propertyWidgets())
			widget->updatePropertyValue();
	}
	return result != 0;
}

void DialogImpl::show()
{
	completeLayout();
	m_dialog->show();
}

void DialogImpl::close()
{
	m_dialog->reject();
}

void DialogImpl::completeLayout()
{
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
										  QDialogButtonBox::Cancel);

	QObject::connect(buttonBox, SIGNAL(accepted()), m_dialog, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), m_dialog, SLOT(reject()));

	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(m_panelLayout);
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	auto layout = m_dialog->layout();
	if(layout)
		delete layout;

	m_dialog->setLayout(mainLayout);
}

/******************************************************************************/

SimpleGUIImpl::SimpleGUIImpl(MainWindow* mainWindow)
	: m_mainWindow(mainWindow)
	, m_buttonsPanel(mainWindow, mainWindow->buttonsLayout())
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
	layout = new QGridLayout(buttonsWidget);
	m_buttonsPanel = PanelImpl(m_mainWindow, layout);

	// Dialogs
	for(auto dialog : m_dialogs)
		dialog->close();
	m_dialogs.clear();
}

ui::SimpleGUI::DialogPtr SimpleGUIImpl::createDialog(const std::string& title)
{
	auto dialog = std::make_shared<DialogImpl>(m_mainWindow, title);
	m_dialogs.push_back(dialog);
	return dialog;
}
