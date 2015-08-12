#include <ui/PropertiesDialog.h>
#include <ui/MainWindow.h>
#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

#include <iostream>

template <class T>
QWidget* createPropWidget(const Property::PropertyPtr& prop, QWidget* parent)
{
	auto propValue = prop->value<T>();
	if(!propValue)
		return nullptr;
	auto propWidget = new PropertyWidget<T>(prop, propValue, parent);
	return propWidget->createWidgets(prop->readOnly());
}

PropertiesDialog::PropertiesDialog(std::shared_ptr<ObjectProperties> objectProperties, MainWindow* mainWindow)
	: QDialog(mainWindow)
	, m_mainWindow(mainWindow)
	, m_objectProperties(objectProperties)
{
	setMinimumSize(300, 200);
	resize(500, 600);
	setWindowTitle(m_objectProperties->name().c_str());
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	auto tabWidget = new QTabWidget;
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply	|
										  QDialogButtonBox::Cancel	|
										  QDialogButtonBox::Reset	|
										  QDialogButtonBox::Ok);

	auto applyButton = buttonBox->button(QDialogButtonBox::Apply);
	auto resetButton = buttonBox->button(QDialogButtonBox::Reset);
	auto OkButton = buttonBox->button(QDialogButtonBox::Ok);
	connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(apply()));
	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(resetWidgets()));
	connect(OkButton, SIGNAL(clicked(bool)), this, SLOT(applyAndClose()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	connect(this, SIGNAL(finished(int)), this, SLOT(removeSelf()));

	auto mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	std::map<std::string, std::vector<int>> propertyGroups;
	m_propertyWidgets.reserve(m_objectProperties->properties().size());

	// Create the property widgets
	for(const auto& prop : m_objectProperties->properties())
	{
		// create property type specific widget
		std::shared_ptr<BasePropertyWidget> propWidget = PropertyWidgetFactory::instance().create(prop, this);
		if(propWidget)
		{
			connect(propWidget.get(), SIGNAL(stateChanged(BasePropertyWidget*,int)), this, SLOT(stateChanged(BasePropertyWidget*,int)));
			int id  = m_propertyWidgets.size();
			propertyGroups[prop->group()].push_back(id);

			PropertyStruct propStruct;
			propStruct.property = prop;
			propStruct.widget = propWidget;
			m_propertyWidgets.push_back(propStruct);
		}
		else
		{
			std::cerr << "Couldn't create a widget for the property " << prop->name() << std::endl;
		}
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

void PropertiesDialog::addTab(QTabWidget* tabWidget, QString name, IntListIter begin, IntListIter end)
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
		auto& prop = m_propertyWidgets[*it];
		auto groupBox = new QGroupBox;
		auto layout = new QVBoxLayout;
		layout->setContentsMargins(5, 5, 5, 5);
		groupBox->setLayout(layout);
		QString title = prop.property->name().c_str();
		groupBox->setTitle(title);
		layout->addWidget(prop.widget->createWidgets());
		scrollLayout->addWidget(groupBox);

		prop.groupBox = groupBox;
		prop.title = title;
	}

	scrollLayout->addStretch(1);

	tabWidget->addTab(scrollArea, name);
}

void PropertiesDialog::apply()
{
	writeToProperties();
	m_objectProperties->apply();
}

void PropertiesDialog::applyAndClose()
{
	writeToProperties();
	m_objectProperties->apply();
	accept();
}

void PropertiesDialog::resetWidgets()
{
	for(auto& widget : m_propertyWidgets)
	{
		widget.widget->resetWidget();
		widget.widget->setWidgetDirty();
	}
}

void PropertiesDialog::removeSelf()
{
	m_mainWindow->removeDialog(this);
}

void PropertiesDialog::writeToProperties()
{
	for(auto widget : m_propertyWidgets)
		widget.widget->updatePropertyValue();
}

void PropertiesDialog::readFromProperties()
{
	for(auto widget : m_propertyWidgets)
		widget.widget->updateWidgetValue();
}

void PropertiesDialog::stateChanged(BasePropertyWidget* widget, int stateVal)
{
	auto it = std::find_if(m_propertyWidgets.begin(), m_propertyWidgets.end(), [widget](const PropertyStruct& prop) {
		return prop.widget.get() == widget;
	});
	if(it == m_propertyWidgets.end())
		return;

	auto groupBox = it->groupBox;
	const auto& title = it->title;
	auto state = static_cast<BasePropertyWidget::State>(stateVal);
	switch(state)
	{
	case BasePropertyWidget::State::unchanged:
		groupBox->setTitle(title);
		break;
	case BasePropertyWidget::State::modified:
		groupBox->setTitle("* " + title);
		break;
	case BasePropertyWidget::State::conflict:
		groupBox->setTitle("! " + title);
		break;
	}
}
