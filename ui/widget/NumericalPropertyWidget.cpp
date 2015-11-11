#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <QtWidgets>

template <class value_type>
class SpinBoxPropertyWidgetContainer
{
protected:
	QSpinBox* m_spinBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_spinBox = new QSpinBox(parent);
		m_spinBox->setMinimum(INT_MIN);
		m_spinBox->setMaximum(INT_MAX);
		m_spinBox->setSingleStep(1);
		m_spinBox->setEnabled(!parent->readOnly());

		void(QSpinBox::*valueChanged)(int) = &QSpinBox::valueChanged; // Get the right overload
		QObject::connect(m_spinBox, valueChanged, parent, &BasePropertyWidget::setWidgetDirty);

		return m_spinBox;
	}
	void readFromProperty(const value_type& v)
	{
		QSignalBlocker block(m_spinBox);
		if(v != m_spinBox->value())
			m_spinBox->setValue(v);
	}
	void writeToProperty(value_type& v)
	{
		v = m_spinBox->value();
	}
};

template<> class PropretyWidgetContainer<int> : public SpinBoxPropertyWidgetContainer<int>{};
template<> class PropretyWidgetContainer<unsigned int> : public SpinBoxPropertyWidgetContainer<unsigned int>{};

//****************************************************************************//

template <class value_type>
class CheckboxPropertyWidget : public PropretyWidgetContainer<value_type>
{
protected:
	QCheckBox* m_checkBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_checkBox = new QCheckBox(parent);
		m_checkBox->setEnabled(!parent->readOnly());
		QObject::connect(m_checkBox, &QCheckBox::clicked, parent, &BasePropertyWidget::setWidgetDirty);
		return m_checkBox;
	}
	void readFromProperty(const value_type& v)
	{
		bool b = (v!=0);
		if (m_checkBox->isChecked() != b)
			m_checkBox->setChecked(b);
	}
	void writeToProperty(value_type& v)
	{
		v = (m_checkBox->isChecked() ? 1 : 0);
	}
};

//****************************************************************************//

template <class value_type>
class SliderPropertyWidget : public PropretyWidgetContainer<value_type>
{
protected:
	QSlider* m_slider = nullptr;
	float m_scale = 1.f, m_offset = 0;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_slider = new QSlider(parent);
		m_slider->setOrientation(Qt::Horizontal);
		m_slider->setEnabled(!parent->readOnly());
		QObject::connect(m_slider, &QSlider::valueChanged, parent, &BasePropertyWidget::setWidgetDirty);

		auto rangeMeta = parent->property()->getMeta<meta::RangeWithStep>();
		float minVal = 0, maxVal = 100, step = 1;
		if (rangeMeta)
		{
			minVal = rangeMeta->min;
			maxVal = rangeMeta->max;
			step = rangeMeta->step;
		}

		m_scale = 1 / step;
		m_offset = -minVal * m_scale;

		float range = maxVal - minVal;
		int intRange = static_cast<int>(range * m_scale);
		m_slider->setMinimum(0);
		m_slider->setMaximum(intRange);

		return m_slider;
	}
	void readFromProperty(const value_type& v)
	{
		QSignalBlocker block(m_slider);
		m_slider->setValue(static_cast<int>(m_offset + v * m_scale));
	}
	void writeToProperty(value_type& v)
	{
		v = static_cast<value_type>((m_slider->value() - m_offset) / m_scale);
	}
};

//****************************************************************************//

template <class value_type>
class EnumIntPropertyWidget : public PropretyWidgetContainer<value_type>
{
protected:
	QComboBox* m_comboBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_comboBox = new QComboBox(parent);
		m_comboBox->setEnabled(!parent->readOnly());
		auto enumMeta = parent->property()->getMeta<meta::Enum>();
		if (enumMeta)
		{
			for (const auto& v : enumMeta->values)
				m_enumValues.push_back(QString::fromStdString(v));
		}
		m_comboBox->addItems(m_enumValues);

		void(QComboBox::*currentIndexChanged)(int) = &QComboBox::currentIndexChanged; // Get the right overload
		QObject::connect(m_comboBox, currentIndexChanged, parent, &BasePropertyWidget::setWidgetDirty);
		return m_comboBox;
	}
	void readFromProperty(const value_type& v)
	{
		QSignalBlocker block(m_comboBox);
		if (m_comboBox->currentIndex() != v)
			m_comboBox->setCurrentIndex(v);
	}
	void writeToProperty(value_type& v)
	{
		v = m_comboBox->currentIndex();
	}

protected:
	QStringList m_enumValues;
};


//****************************************************************************//

template <>
class PropretyWidgetContainer<float>
{
protected:
	using value_type = float;
	QLineEdit* m_lineEdit = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, &QLineEdit::editingFinished, parent, &BasePropertyWidget::setWidgetDirty);
		return m_lineEdit;
	}
	void readFromProperty(const value_type& v)
	{
		QString t = m_lineEdit->text();
		value_type n = t.toFloat();

		if (v != n || t.isEmpty())
			m_lineEdit->setText(QString::number(v));
	}
	void writeToProperty(value_type& v)
	{
		bool ok;
		value_type n = m_lineEdit->text().toFloat(&ok);

		if(ok)
			v = n;
	}
};

//****************************************************************************//

template <>
class PropretyWidgetContainer<double>
{
protected:
	using value_type = double;
	QLineEdit* m_lineEdit = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, &QLineEdit::editingFinished, parent, &BasePropertyWidget::setWidgetDirty);
		return m_lineEdit;
	}
	void readFromProperty(const value_type& v)
	{
		QString t = m_lineEdit->text();
		value_type n = t.toDouble();

		if (v != n || t.isEmpty())
			m_lineEdit->setText(QString::number(v));
	}
	void writeToProperty(value_type& v)
	{
		bool ok;
		value_type n = m_lineEdit->text().toDouble(&ok);

		if(ok)
			v = n;
	}
};

//****************************************************************************//

RegisterWidget<SimplePropertyWidget<bool, CheckboxPropertyWidget<bool>> > PW_bool("default");

RegisterWidget<SimplePropertyWidget<int>> PW_int("default");
RegisterWidget<SimplePropertyWidget<int, CheckboxPropertyWidget<int>> > PW_checkbox("checkbox");
RegisterWidget<SimplePropertyWidget<int, EnumIntPropertyWidget<int>> > PW_int_enum("enum");
RegisterWidget<SimplePropertyWidget<int, SliderPropertyWidget<int>> > PW_int_slider("slider");

RegisterWidget<SimplePropertyWidget<unsigned int>> PW_unsigned_int("default");
RegisterWidget<SimplePropertyWidget<unsigned int, CheckboxPropertyWidget<unsigned int>> > PW_unsigned_checkbox("checkbox");
RegisterWidget<SimplePropertyWidget<unsigned int, EnumIntPropertyWidget<unsigned int>> > PW_unsigned_enum("enum");
RegisterWidget<SimplePropertyWidget<unsigned int, SliderPropertyWidget<unsigned int>> > PW_unsigned_slider("slider");

RegisterWidget<SimplePropertyWidget<float>> PW_float("default");
RegisterWidget<SimplePropertyWidget<float, SliderPropertyWidget<float>> > PW_float_slider("slider");

RegisterWidget<SimplePropertyWidget<double>> PW_double("default");
RegisterWidget<SimplePropertyWidget<double, SliderPropertyWidget<double>> > PW_double_slider("slider");
