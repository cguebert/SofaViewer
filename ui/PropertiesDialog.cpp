#include <ui/PropertiesDialog.h>
#include <ui/PropertyWidget.h>

#include <core/ObjectProperties.h>

#include <sfe/Data.h>

#include <QtWidgets>

template <class T>
QWidget* createPropWidget(const Property& prop, QWidget* parent)
{
	auto propValue = std::dynamic_pointer_cast<PropertyValue<T>>(prop.m_value);
	if(!propValue)
		return nullptr;
	auto propWidget = new PropertyWidget<T>(propValue, parent);
	return propWidget->createWidgets(prop.readOnly());
}

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
		using DataType = sfe::Data::DataType;
		auto type = static_cast<DataType>(prop.m_valueType);
		QWidget* propWidget = nullptr;
		switch(type)
		{
		case DataType::Int:
			propWidget = createPropWidget<int>(prop, this);
			break;
		}

		if(propWidget)
		{
			layout->addWidget(propWidget);
		}
		else
		{
			auto edit = new QLineEdit;
			edit->setText(prop.m_stringValue.c_str());
			layout->addWidget(edit);
		}

		scrollLayout->addWidget(group);
	}

	setLayout(layout);
}
