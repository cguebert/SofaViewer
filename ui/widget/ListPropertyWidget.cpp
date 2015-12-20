#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <core/PropertiesUtils.h>

#include <QtWidgets>

#include <cassert>

namespace
{

// Used to differentiate treatment for std::vector and VectorWrapper
template <class T> class ListTraits;

template <class T>
class ListTraits<std::vector<T>>
{
public:
	using value_type = std::vector<T>;
	using base_type = T;

	static int size(const value_type& value) { return value.size(); }
	static void resize(value_type& value, int nb) { value.resize(nb); }
	static bool fixed(const value_type& value) { return false; }
	static base_type& value(value_type& value, int index) { return value[index]; }
};

template <class T>
class ListTraits<VectorWrapper<T>>
{
public:
	using value_type = VectorWrapper<T>;
	using base_type = typename value_type::base_type;

	static int size(const value_type& wrapper) { return wrapper.value().size(); }
	static void resize(value_type& wrapper, int nb) { wrapper.value().resize(nb); }
	static bool fixed(const value_type& wrapper) { return wrapper.fixedSize(); }
	static base_type& value(value_type& wrapper, int index) { return wrapper.value()[index]; }
};

}

template <class T>
class ListWidgetContainer
{
protected:
	using value_type = T;
	using list_traits = ListTraits<value_type>;
	using base_type = typename list_traits::base_type;

	QSpinBox* m_spinBox = nullptr;
	QPushButton* m_toggleButton = nullptr;
	QScrollArea* m_scrollArea = nullptr;
	QFormLayout* m_formLayout = nullptr;
	const BasePropertyWidgetCreator* m_widgetCreator;
	Property::SPtr m_property;
	std::shared_ptr<PropertyValue<value_type>> m_propertyValue;
	std::vector<std::shared_ptr<BasePropertyWidget>> m_propertyWidgets;
	value_type m_value;
	BasePropertyWidget* m_parent;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_parent = parent;
		m_property = parent->property();
		m_propertyValue = m_property->value<value_type>();
		if (!m_propertyValue)
			return nullptr;
		m_value = m_propertyValue->value();

		// Get widget creator
		std::string widget;
		auto meta = m_propertyValue->metaContainer().get<meta::Widget>();
		if (meta)
			widget = meta->type();

		auto id = std::type_index(typeid(base_type));
		m_widgetCreator = PropertyWidgetFactory::instance().creator(id, widget);

		auto container = new QWidget(parent);
		auto layout = new QVBoxLayout(container);
		layout->setContentsMargins(0, 0, 0, 0);

		auto topLayout = new QHBoxLayout;
		m_toggleButton = new QPushButton(QPushButton::tr("show"));
		m_toggleButton->setCheckable(true);
		QObject::connect(m_toggleButton, &QPushButton::toggled, [this](bool toggled){ toggleView(toggled); });
		topLayout->addWidget(m_toggleButton);

		if (!list_traits::fixed(m_value))
		{
			m_spinBox = new QSpinBox;
			m_spinBox->setMaximum(INT_MAX);
			m_spinBox->setValue(list_traits::size(m_value));
			topLayout->addWidget(m_spinBox, 1);

			auto resizeButton = new QPushButton(QPushButton::tr("resize"));
			QObject::connect(resizeButton, &QPushButton::clicked, [this]() { 
				for (auto w : m_propertyWidgets) 
					w->updatePropertyValue(); 
				resize(); 
			});
			QObject::connect(resizeButton, &QPushButton::clicked, parent, &BasePropertyWidget::setWidgetDirty);
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
	void readFromProperty(const value_type& v)
	{
		m_value = v;
		int prevNb = m_spinBox->value();
		int nb = list_traits::size(m_value);

		if (prevNb != nb)
		{
			if (m_spinBox)
				m_spinBox->setValue(nb);

			resize();
		}
		else
		{
			for (auto w : m_propertyWidgets)
				w->updateWidgetValue();
		}
	}
	void writeToProperty(value_type& v)
	{
		for (auto w : m_propertyWidgets)
			w->updatePropertyValue();

		v = m_value;
	}
	void resize()
	{
		int nb = m_spinBox->value();
		int prevNb = list_traits::size(m_value);

		if (m_formLayout && nb == prevNb)
			return; // No need to recreate the same widgets

		bool visible = m_scrollArea->isVisible();
		m_scrollArea->setVisible(false);
		m_propertyWidgets.clear();

		list_traits::resize(m_value, nb);
			
		auto scrollAreaWidget = new QWidget;
		m_formLayout = new QFormLayout;
		m_formLayout->setContentsMargins(3, 3, 3, 3);

		auto name = m_property->name();
		const auto& metaProperties = m_propertyValue->metaContainer().properties();

		for (int i = 0; i < nb; ++i)
		{
			auto& value = list_traits::value(m_value, i);
			auto prop = property::createRefProperty(name, value);
			auto propValue = prop->value<base_type>();
			auto& metaContainer = propValue->metaContainer();
			for (auto& metaProp : metaProperties)
				metaContainer.addExisting(metaProp);

			std::shared_ptr<BasePropertyWidget> propWidget = m_widgetCreator->create(prop, scrollAreaWidget);
			propWidget->setParent(m_parent);
			m_propertyWidgets.push_back(propWidget);
			m_formLayout->addRow(QString::number(i), propWidget->createWidgets());
		}

		scrollAreaWidget->setLayout(m_formLayout);
		m_scrollArea->setWidget(scrollAreaWidget);
		if (visible)
			m_scrollArea->setVisible(true);
	}
	void toggleView(bool show)
	{
		if (show && !m_formLayout)
			resize();
		m_scrollArea->setVisible(show);
		m_toggleButton->setText(show ? QPushButton::tr("hide") : QPushButton::tr("show"));
	}
};

/*****************************************************************************/

template <class T>
class RegisterListWidget
{
public:
	template <class U> using ListPropertyWidget = SimplePropertyWidget<U, ListWidgetContainer<U>>;

	explicit RegisterListWidget(const std::string& widgetName)
	{
		using vector_type = std::vector<T>;
		RegisterWidget<ListPropertyWidget<vector_type>> PW_vector(widgetName);
		RegisterWidget<ListPropertyWidget<VectorWrapper<vector_type>>> PW_vector_wrapper(widgetName);
	}
};
/*****************************************************************************/

RegisterListWidget<int> PW_generic_int("generic");
RegisterListWidget<unsigned int> PW_generic_unsigned_int("generic");
RegisterListWidget<float> PW_generic_float("generic");
RegisterListWidget<double> PW_generic_double("generic");
RegisterListWidget<std::string> PW_generic_string("generic");
