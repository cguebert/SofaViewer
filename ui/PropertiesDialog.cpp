#include <ui/PropertiesDialog.h>
#include <ui/PropertyWidget.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

template <class T>
QWidget* createPropWidget(const ObjectProperties::PropertyPtr& prop, QWidget* parent)
{
	auto propValue = std::dynamic_pointer_cast<PropertyValue<T>>(prop->value());
	if(!propValue)
		return nullptr;
	auto propWidget = new PropertyWidget<T>(prop, propValue, parent);
	return propWidget->createWidgets(prop->readOnly());
}

PropertiesDialog::PropertiesDialog(std::shared_ptr<ObjectProperties> properties, QWidget* parent)
	: QDialog(parent)
	, m_properties(properties)
{
	setMinimumSize(300, 200);
	resize(500, 600);
	setWindowTitle(properties->objectName().c_str());

	auto tabWidget = new QTabWidget;
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply |
										  QDialogButtonBox::Reset |
										  QDialogButtonBox::Ignore);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(reject()));

	auto mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	std::map<std::string, std::vector<PropertyPair>> propertyGroups;

	// Create the property widgets
	for(const auto& prop : properties->properties())
	{
		// create property type specific widget
		QWidget* propWidget = nullptr;
		switch(prop->storageType())
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

		if(!propWidget)
		{
			auto lineEdit = new QLineEdit;
			lineEdit->setText(prop->m_stringValue.c_str());
			lineEdit->setEnabled(prop->readOnly());

			propWidget = lineEdit;
		}

		PropertyPair propPair = std::make_pair(prop, propWidget);
		m_propertyWidgets.push_back(propPair);
		propertyGroups[prop->group()].push_back(propPair);
	}

	// Group the widgets and add them to the dialog
	for(const auto& group : propertyGroups)
	{
		QString name = group.first.c_str();
		if(name.isEmpty())
			name = "Property";

		auto scrollArea = new QScrollArea;
		scrollArea->setFrameStyle(QFrame::NoFrame);
		auto scrollWidget = new QWidget;
		auto scrollLayout = new QVBoxLayout;
		scrollWidget->setLayout(scrollLayout);
		scrollLayout->setContentsMargins(0, 0, 0, 0);
		scrollArea->setWidget(scrollWidget);
		scrollArea->setWidgetResizable(true);

		for(const auto& propPair : group.second)
		{
			auto groupBox = new QGroupBox;
			auto layout = new QVBoxLayout;
			layout->setContentsMargins(5, 5, 5, 5);
			groupBox->setLayout(layout);
			groupBox->setTitle(propPair.first->name().c_str());
			layout->addWidget(propPair.second);
			scrollLayout->addWidget(groupBox);
		}

		tabWidget->addTab(scrollArea, name);
	}

	setLayout(mainLayout);
}
