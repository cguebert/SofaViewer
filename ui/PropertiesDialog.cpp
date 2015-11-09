#include <ui/PropertiesDialog.h>
#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

#include <iostream>

PropertiesDialog::PropertiesDialog(std::shared_ptr<ObjectProperties> objectProperties, GraphNode* node, QWidget* parent)
	: QDialog(parent)
	, m_graphNode(node)
	, m_objectProperties(objectProperties)
{
	setMinimumSize(300, 200);
	resize(500, 600);
	setWindowTitle(QString::fromStdString(m_objectProperties->name()));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	auto tabWidget = new QTabWidget;
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply	|
										  QDialogButtonBox::Cancel	|
										  QDialogButtonBox::Reset	|
										  QDialogButtonBox::Ok);

	auto applyButton = buttonBox->button(QDialogButtonBox::Apply);
	auto resetButton = buttonBox->button(QDialogButtonBox::Reset);
	auto OkButton = buttonBox->button(QDialogButtonBox::Ok);
	connect(applyButton, &QPushButton::clicked, this, &PropertiesDialog::apply);
	connect(resetButton, &QPushButton::clicked, this, &PropertiesDialog::resetWidgets);
	connect(OkButton, &QPushButton::clicked, this, &PropertiesDialog::applyAndClose);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &PropertiesDialog::reject);

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
			connect(propWidget.get(), &BasePropertyWidget::stateChanged, this, &PropertiesDialog::stateChanged);
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
		QString name = QString::fromStdString(group.first);
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

	m_objectProperties->addModifiedCallback([this](){
		readFromProperties();
	});
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
		QString title = QString::fromStdString(prop.property->name());
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
	doApply();
}

void PropertiesDialog::applyAndClose()
{
	if(doApply())
		accept();
}

bool PropertiesDialog::doApply()
{
	bool conflict = false;
	for(auto& widget : m_propertyWidgets)
	{
		if(widget.widget->state() == BasePropertyWidget::State::conflict)
		{
			conflict = true;
			break;
		}
	}

	if(conflict)
	{
		auto result = QMessageBox::warning(this, "Resolve conflicts",
										   "At least one property is in conflict. Resolve using user values ?",
										   QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
		if(result == QMessageBox::Cancel)
			return false;
	}

	writeToProperties();
	m_objectProperties->applyProperties();
	return true;
}

void PropertiesDialog::resetWidgets()
{
	for(auto& widget : m_propertyWidgets)
	{
		widget.widget->resetWidget();
		widget.widget->setWidgetDirty();
	}
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
