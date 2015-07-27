#include <ui/PropertiesDialog.h>
#include <ui/PropertyWidget.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

template <class T>
QWidget* createPropWidget(const Property& prop, QWidget* parent)
{
	auto propValue = std::dynamic_pointer_cast<PropertyValue<T>>(prop.value());
	if(!propValue)
		return nullptr;
	auto propWidget = new PropertyWidget<T>(prop, propValue, parent);
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
		QWidget* propWidget = nullptr;
		switch(prop.storageType())
		{
		case Property::Type::Int:
			propWidget = createPropWidget<int>(prop, this);
			break;
		case Property::Type::Vector_Int:
			propWidget = createPropWidget<std::vector<int>>(prop, this);
			break;
		case Property::Type::Vector_Float:
			propWidget = createPropWidget<std::vector<float>>(prop, this);
			break;
		case Property::Type::Vector_Double:
			propWidget = createPropWidget<std::vector<double>>(prop, this);
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
