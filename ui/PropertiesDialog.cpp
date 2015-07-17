#include <ui/PropertiesDialog.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

PropertiesDialog::PropertiesDialog(std::shared_ptr<ObjectProperties> properties, QWidget* parent)
	: QDialog(parent)
	, m_properties(properties)
{
	setMinimumSize(300, 200);
	setWindowTitle(properties->m_name.c_str());

	auto layout = new QVBoxLayout;
	layout->setMargin(0);
	auto scroll = new QScrollArea;
	scroll->setFrameStyle(0);
	layout->addWidget(scroll);
	auto scrollWidget = new QWidget;
	auto scrollLayout = new QVBoxLayout;
	scrollWidget->setLayout(scrollLayout);
	scrollLayout->setMargin(0);
	scroll->setWidget(scrollWidget);
	scroll->setWidgetResizable(true);

	for(const auto& prop : properties->m_properties)
	{
		auto group = new QGroupBox;
		auto layout = new QVBoxLayout;
		layout->setMargin(5);
		group->setLayout(layout);
		group->setTitle(prop.name().c_str());

		// create property type specific widget
		auto edit = new QLineEdit;
		edit->setText(prop.m_value.c_str());
		layout->addWidget(edit);
		scrollLayout->addWidget(group);
	}

	setLayout(layout);
}
