#include <ui/MainWindow.h>
#include <ui/simplegui/PanelImpl.h>
#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <QtWidgets>

#include <iostream>

PanelImpl::PanelImpl(MainWindow* mainWindow, QGridLayout* layout)
	: m_mainWindow(mainWindow)
	, m_layout(layout)
{

}

void PanelImpl::addButton(const std::string& name, const std::string& help,
							 simplegui::CallbackFunc callback,
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

void PanelImpl::addProperty(Property::SPtr property,
							int row, int column,
							int rowSpan, int columnSpan)
{
	std::shared_ptr<BasePropertyWidget> propWidget = PropertyWidgetFactory::instance().create(property, m_mainWindow);
	if(!propWidget)
	{
		std::cerr << "Could not create a property widget for " << property->name() << std::endl;
		return;
	}

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
