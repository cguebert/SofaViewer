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

		QObject::connect(m_spinBox, &QSpinBox::editingFinished, parent, &BasePropertyWidget::setWidgetDirty);

		return m_spinBox;
	}
	void readFromProperty(const value_type& v)
	{
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
class EnumIntPropertyWidget : public PropretyWidgetContainer<value_type>
{
protected:
	QComboBox* m_comboBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_comboBox = new QComboBox(parent);
		m_comboBox->setEnabled(!parent->readOnly());
		auto enumMeta = parent->property()->meta()->get<meta::Enum>();
		if (enumMeta)
		{
			for (const auto& v : enumMeta->values)
				m_enumValues.push_back(v.c_str());
		}
		m_comboBox->addItems(m_enumValues);

		QObject::connect(m_comboBox, &QComboBox::setCurrentIndex, parent, &BasePropertyWidget::setWidgetDirty);
		return m_comboBox;
	}
	void readFromProperty(const value_type& v)
	{
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

RegisterWidget<SimplePropertyWidget<int>> PW_int("default");
RegisterWidget<SimplePropertyWidget<int, CheckboxPropertyWidget<int>> > PW_checkbox("checkbox");
RegisterWidget<SimplePropertyWidget<int, EnumIntPropertyWidget<int>> > PW_int_enum("enum");
RegisterWidget<SimplePropertyWidget<unsigned int>> PW_unsigned_int("default");
RegisterWidget<SimplePropertyWidget<unsigned int, CheckboxPropertyWidget<unsigned int>> > PW_unsigned_checkbox("checkbox");
RegisterWidget<SimplePropertyWidget<unsigned int, EnumIntPropertyWidget<unsigned int>> > PW_unsigned_enum("enum");
RegisterWidget<SimplePropertyWidget<float>> PW_float("default");
RegisterWidget<SimplePropertyWidget<double>> PW_double("default");
