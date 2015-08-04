#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <QtWidgets>

template<>
class PropretyWidgetContainer<int>
{
protected:
	typedef int value_type;
	QSpinBox* m_spinBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_spinBox = new QSpinBox(parent);
		m_spinBox->setMinimum(INT_MIN);
		m_spinBox->setMaximum(INT_MAX);
		m_spinBox->setSingleStep(1);
		m_spinBox->setEnabled(!parent->readOnly());

		QObject::connect(m_spinBox, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));

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

//****************************************************************************//

class CheckboxPropertyWidget : public PropretyWidgetContainer<int>
{
protected:
	typedef int value_type;
	QCheckBox* m_checkBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_checkBox = new QCheckBox(parent);
		m_checkBox->setEnabled(!parent->readOnly());
		QObject::connect(m_checkBox, SIGNAL(clicked(bool)), parent, SLOT(setWidgetDirty()));
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

template <>
class PropretyWidgetContainer<float>
{
protected:
	typedef float value_type;
	QLineEdit* m_lineEdit = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
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
	typedef double value_type;
	QLineEdit* m_lineEdit = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
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
RegisterWidget<SimplePropertyWidget<int, CheckboxPropertyWidget> > PW_checkbox("checkbox");
RegisterWidget<SimplePropertyWidget<float>> PW_float("default");
RegisterWidget<SimplePropertyWidget<double>> PW_double("default");
