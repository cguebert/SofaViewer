#include <ui/widget/StructPropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <QtWidgets>

StructPropertyWidget::StructPropertyWidget(Property::SPtr prop, QWidget* parent)
	: BasePropertyWidget(prop, parent)
	, m_structProperty(m_property->getMeta<meta::Struct>())
{
	// Get widget creators
	for (const auto& item : m_structProperty->items)
	{
		std::string widget;
		auto meta = item->metaContainer().get<meta::Widget>();
		if (meta)
			widget = meta->type();
		m_widgetCreators.push_back(PropertyWidgetFactory::instance().creator(item->type(), widget));
	}

	m_value = m_structProperty->cloneProperty(m_property);
	m_resetValue = m_structProperty->cloneProperty(m_property);
}

QWidget* StructPropertyWidget::createWidgets()
{
	if (!m_structProperty)
		return nullptr;

	auto parent = parentWidget();
	auto container = new QWidget(parent);
	auto layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);

	auto topLayout = new QHBoxLayout;
	m_toggleButton = new QPushButton(QPushButton::tr("show"));
	m_toggleButton->setCheckable(true);
	QObject::connect(m_toggleButton, &QPushButton::toggled, [this](bool toggled){ toggleView(toggled); });
	topLayout->addWidget(m_toggleButton);

	if (!m_structProperty->isFixedSize(m_value))
	{
		m_spinBox = new QSpinBox;
		m_spinBox->setMaximum(INT_MAX);
		m_spinBox->setValue(m_structProperty->getSize(m_value));
		topLayout->addWidget(m_spinBox, 1);

		auto resizeButton = new QPushButton(QPushButton::tr("resize"));
		QObject::connect(resizeButton, &QPushButton::clicked, [this]() { writeToProperty(m_value); resize(m_spinBox->value()); });
		QObject::connect(resizeButton, &QPushButton::clicked, [this]() { setWidgetDirty(); });
		topLayout->addWidget(resizeButton);
	}
	else
		topLayout->addStretch();
		
	m_scrollArea = new QScrollArea();

	layout->addLayout(topLayout);
	layout->addWidget(m_scrollArea, 1);

	m_scrollArea->hide();
	m_scrollArea->setWidgetResizable(true);

	return container;
}

void StructPropertyWidget::readFromProperty(BasePropertyValue::SPtr value)
{
	m_structProperty->setValue(m_value, value);
	int prevNb = m_spinBox->value();
	int nb = m_structProperty->getSize(m_value);

	if (prevNb != nb)
	{
		m_spinBox->setValue(nb);
		resize(nb);
	}
	else
	{
		for (auto w : m_propertyWidgets)
			w->resolveConflict(Source::property);
	}
}

void StructPropertyWidget::writeToProperty(BasePropertyValue::SPtr value)
{
	for (auto w : m_propertyWidgets)
		w->updatePropertyValue();

	if(value != m_value)
		m_structProperty->setValue(value, m_value);
}

void StructPropertyWidget::readFromProperty()
{
	readFromProperty(m_property->value());
}

void StructPropertyWidget::writeToProperty()
{
	writeToProperty(m_resetValue);
	m_structProperty->setValue(m_property->value(), m_resetValue);
}

bool StructPropertyWidget::isModified()
{
	writeToProperty(m_value);
	return m_structProperty->isModified(m_value, m_resetValue);
}

void StructPropertyWidget::resetWidget()
{
	readFromProperty(m_resetValue);
}

void StructPropertyWidget::validate()
{
	auto tempValue = m_structProperty->cloneProperty(m_property);
	writeToProperty(tempValue);
	if (m_structProperty->validate(tempValue))
		readFromProperty(tempValue);
}

void StructPropertyWidget::resize(int nb)
{
	if (nb == -1)
		nb = m_structProperty->getSize(m_value);
	int nbStructItems = m_structProperty->items.size();
	int prevNb = m_propertyWidgets.size() / nbStructItems;

	if (m_formLayout && nb == prevNb)
		return; // No need to recreate the same widgets

	bool visible = m_scrollArea->isVisible();
	m_scrollArea->setVisible(false);
	m_propertyWidgets.clear();

	m_structProperty->resize(m_value, nb);
			
	auto scrollAreaWidget = new QWidget;
	m_formLayout = new QFormLayout;
	m_formLayout->setContentsMargins(3, 3, 3, 3);

	for (int i = 0; i < nb; ++i)
	{
		auto container = new QWidget;
		auto subLayout = new QFormLayout;
		for (int j = 0; j < nbStructItems; ++j)
		{
			auto prop = m_structProperty->getSubProperty(m_value, i, j);
			std::shared_ptr<BasePropertyWidget> propWidget = m_widgetCreators[j]->create(prop, container);
			propWidget->setParent(this);
			m_propertyWidgets.push_back(propWidget);
			subLayout->addRow(QString::fromStdString(prop->name()), propWidget->createWidgets());
		}
		container->setLayout(subLayout);
		m_formLayout->addRow(QString::number(i), container);
	}

	scrollAreaWidget->setLayout(m_formLayout);
	m_scrollArea->setWidget(scrollAreaWidget);
	if (visible)
		m_scrollArea->setVisible(true);

	setWidgetDirty();
}

void StructPropertyWidget::toggleView(bool show)
{
	if (show && !m_formLayout)
		resize(-1);
	m_scrollArea->setVisible(show);
	m_toggleButton->setText(show ? QPushButton::tr("hide") : QPushButton::tr("show"));
}
