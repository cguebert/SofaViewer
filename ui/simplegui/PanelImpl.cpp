#include <ui/simplegui/PanelImpl.h>
#include <ui/simplegui/ButtonImpl.h>

#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <QtWidgets>

#include <iostream>

PanelImpl::PanelImpl(QWidget* parent, QGridLayout* layout)
	: m_parent(parent)
	, m_layout(layout)
{

}

simplegui::Button::SPtr PanelImpl::addButton(const std::string& name, const std::string& help,
											 simplegui::CallbackFunc callback,
											 int row, int column,
											 int rowSpan, int columnSpan)
{
	auto button = new QPushButton(QString::fromStdString(name));
	button->setStatusTip(QString::fromStdString(help));

	QObject::connect(button, &QPushButton::clicked, callback);
	if(row < 0)
		row = m_layout->count() ? m_layout->rowCount() : 0;

	m_layout->addWidget(button, row, column, rowSpan, columnSpan);

	return std::make_shared<PushButton_ButtonImpl>(button);
}

void PanelImpl::addProperty(Property::SPtr property,
							int row, int column,
							int rowSpan, int columnSpan)
{
	std::shared_ptr<BasePropertyWidget> propWidget = PropertyWidgetFactory::instance().create(property, m_parent);
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
		auto containerLayout = new QFormLayout;
		auto label = new QLabel(QString::fromStdString(property->name()));
		containerLayout->addRow(QString::fromStdString(property->name()), widget);
		m_layout->addLayout(containerLayout, row, column, rowSpan, columnSpan);
	}
	else
		m_layout->addWidget(widget, row, column, rowSpan, columnSpan);
}

void PanelImpl::addLabel(const std::string& text,
						 int row, int column,
						 int rowSpan, int columnSpan)
{
	auto label = new QLabel(QString::fromStdString(text));

	if (row < 0)
		row = m_layout->count() ? m_layout->rowCount() : 0;
	m_layout->addWidget(label, row, column, rowSpan, columnSpan);
}
