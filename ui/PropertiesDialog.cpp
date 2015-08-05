#include <ui/PropertiesDialog.h>
#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

template <class T>
QWidget* createPropWidget(const Property::PropertyPtr& prop, QWidget* parent)
{
	auto propValue = std::dynamic_pointer_cast<PropertyValue<T>>(prop->value());
	if(!propValue)
		return nullptr;
	auto propWidget = new PropertyWidget<T>(prop, propValue, parent);
	return propWidget->createWidgets(prop->readOnly());
}

PropertiesDialog::PropertiesDialog(std::shared_ptr<ObjectProperties> objectProperties, QWidget* parent)
	: QDialog(parent)
	, m_objectProperties(objectProperties)
{
	setMinimumSize(300, 200);
	resize(500, 600);
	setWindowTitle(m_objectProperties->objectName().c_str());
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	auto tabWidget = new QTabWidget;
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply |
										  QDialogButtonBox::Reset |
										  QDialogButtonBox::Ok);

	auto resetButton = buttonBox->button(QDialogButtonBox::Reset);
	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(resetWidgets()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(reject()));

	auto mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	std::map<std::string, std::vector<PropertyPair>> propertyGroups;

	// Create the property widgets
	for(const auto& prop : m_objectProperties->properties())
	{
		// create property type specific widget
		std::shared_ptr<BasePropertyWidget> propWidget = PropertyWidgetFactory::instance().create(prop, this);
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

		const int maxPerTab = 10; // TODO: use combined default height of widgets instead
		const auto& properties = group.second;
		const int nb = properties.size();
		if(nb > maxPerTab)
		{
			int nbTabs = (properties.size() + maxPerTab - 1) / maxPerTab;
			auto beginIt = properties.begin();
			for(int i = 0; i < nbTabs; ++i)
			{
				QString tabName = name + " " + QString::number(i+1) + "/" + QString::number(nbTabs);
				auto startDist = i * maxPerTab;
				auto endDist = std::min(nb, startDist + maxPerTab);
				addTab(tabWidget, tabName, beginIt + startDist, beginIt + endDist);
			}
		}
		else
			addTab(tabWidget, name, properties.begin(), properties.end());
	}

	setLayout(mainLayout);
}

void PropertiesDialog::addTab(QTabWidget* tabWidget, QString name, PropertyPairListIter begin, PropertyPairListIter end)
{
	auto scrollArea = new QScrollArea;
	scrollArea->setFrameStyle(QFrame::NoFrame);
	auto scrollWidget = new QWidget;
	auto scrollLayout = new QVBoxLayout;
	scrollWidget->setLayout(scrollLayout);
	scrollLayout->setContentsMargins(0, 0, 0, 0);
	scrollArea->setWidget(scrollWidget);
	scrollArea->setWidgetResizable(true);

	for(auto it = begin; it != end; ++it)
	{
		const auto& propPair = *it;
		auto groupBox = new QGroupBox;
		auto layout = new QVBoxLayout;
		layout->setContentsMargins(5, 5, 5, 5);
		groupBox->setLayout(layout);
		groupBox->setTitle(propPair.first->name().c_str());
		layout->addWidget(propPair.second->createWidgets());
		scrollLayout->addWidget(groupBox);
	}

	scrollLayout->addStretch();

	tabWidget->addTab(scrollArea, name);
}

void PropertiesDialog::resetWidgets()
{
	for(auto& widgetPair : m_propertyWidgets)
		widgetPair.second->resetWidget();
}
